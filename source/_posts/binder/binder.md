---
title: binder
date: 2019-08-27 23:08:04
tags:
---

## 1.libbinder库
Android的framework提供了libbinder库来方便使用binder设备,其文件所在目录为frameworks/native/libs/binder.
其目录结构如下：  
```
dongyus-MBP:libs ruby$ tree -L 2 binder/
binder/
├── ActivityManager.cpp
├── Android.bp
├── AppOpsManager.cpp
├── Binder.cpp
├── BpBinder.cpp
├── BufferedTextOutput.cpp
├── Debug.cpp
├── IActivityManager.cpp
├── IAppOpsCallback.cpp
├── IAppOpsService.cpp
├── IBatteryStats.cpp
├── IInterface.cpp
├── IMediaResourceMonitor.cpp
├── IMemory.cpp
├── IPCThreadState.cpp
├── IPermissionController.cpp
├── IProcessInfoService.cpp
├── IResultReceiver.cpp
├── IServiceManager.cpp
├── IShellCallback.cpp
├── IUidObserver.cpp
├── IpPrefix.cpp
├── MemoryBase.cpp
├── MemoryDealer.cpp
├── MemoryHeapBase.cpp
├── Parcel.cpp
├── PermissionCache.cpp
├── PermissionController.cpp
├── PersistableBundle.cpp
├── ProcessInfoService.cpp
├── ProcessState.cpp
├── Static.cpp
├── Status.cpp
├── TextOutput.cpp
├── Value.cpp
├── aidl
│   └── android
├── include
│   ├── binder
│   └── private
└── tests
    ├── Android.bp
    ├── binderDriverInterfaceTest.cpp
    ├── binderLibTest.cpp
    ├── binderSafeInterfaceTest.cpp
    ├── binderTextOutputTest.cpp
    ├── binderThroughputTest.cpp
    ├── binderValueTypeTest.cpp
    └── schd-dbg.cpp

6 directories, 43 files
```
其中的ProcessState.cpp和IPCThreadState.cpp分别是进程级别的和线程级别的。  
其它I开头的一些文件实现了一些基础的Binder通信客户端和服务端。  

## 2.进程级别对象ProcessState
ProcessState对象一般在进程启动时候去创建，ProcessState对象创建时会去打开binder驱动设备节点，设置最大支持的Binder线程数量，并且通过mmap将binder设备映射到当前进程的内存空间中来。  
```
ProcessState::ProcessState(const char *driver)
    : mDriverName(String8(driver))
    , mDriverFD(open_driver(driver))//打开binder驱动设备节点
    , mVMStart(MAP_FAILED)
    , mThreadCountLock(PTHREAD_MUTEX_INITIALIZER)
    , mThreadCountDecrement(PTHREAD_COND_INITIALIZER)
    , mExecutingThreadsCount(0)
    , mMaxThreads(DEFAULT_MAX_BINDER_THREADS)//设置最大支持的binder线程数量
    , mStarvationStartTimeMs(0)
    , mManagesContexts(false)
    , mBinderContextCheckFunc(NULL)
    , mBinderContextUserData(NULL)
    , mThreadPoolStarted(false)
    , mThreadPoolSeq(1)
{
    if (mDriverFD >= 0) {
        // mmap the binder, providing a chunk of virtual address space to receive transactions.
        mVMStart = mmap(0, BINDER_VM_SIZE, PROT_READ, MAP_PRIVATE | MAP_NORESERVE, mDriverFD, 0);//通过mmap将binder设备映射到当前进程的内存空间。
        if (mVMStart == MAP_FAILED) {
            // *sigh*
            ALOGE("Using %s failed: unable to mmap transaction memory.\n", mDriverName.c_str());
            close(mDriverFD);
            mDriverFD = -1;
            mDriverName.clear();
        }
    }

    LOG_ALWAYS_FATAL_IF(mDriverFD < 0, "Binder driver could not be opened.  Terminating.");
}
```
创建ProcessState对象后，一般再通过startThreadPool()函数，启动一个线程去工作
```
void ProcessState::spawnPooledThread(bool isMain)
{
    if (mThreadPoolStarted) {
        String8 name = makeBinderThreadName();
        ALOGV("Spawning new pooled thread, name=%s\n", name.string());
        sp<Thread> t = new PoolThread(isMain);
        t->run(name.string());
    }
}

void ProcessState::startThreadPool()
{
    AutoMutex _l(mLock);
    if (!mThreadPoolStarted) {
        mThreadPoolStarted = true;
        spawnPooledThread(true);
    }
}
```
我们需要观察下这个PoolThread是干什么的。  

```
class PoolThread : public Thread
{
public:
    explicit PoolThread(bool isMain)
        : mIsMain(isMain)
    {
    }

protected:
    virtual bool threadLoop()
    {
        IPCThreadState::self()->joinThreadPool(mIsMain);
        return false;
    }

    const bool mIsMain;
};
```
可以看到PoolThread线程运行后会创建一个IPCThreadState对象并调用其joinThreadPool函数。

## 3.线程级别对象IPCThreadState
ProcessState创建PoolThread运行后就在执行joinThreadPool().joinThreadPool到底在干什么呢？  
从joinThreadPool的代码我们可以看到这个线程进入了一个循环，在循环中不停接收binder驱动传过来的数据，并处理。
```
status_t IPCThreadState::getAndExecuteCommand()
{
    status_t result;
    int32_t cmd;

    result = talkWithDriver();//从binder驱动读取数据
    if (result >= NO_ERROR) {
        size_t IN = mIn.dataAvail();
        if (IN < sizeof(int32_t)) return result;
        cmd = mIn.readInt32();
        IF_LOG_COMMANDS() {
            alog << "Processing top-level Command: "
                 << getReturnString(cmd) << endl;
        }

        pthread_mutex_lock(&mProcess->mThreadCountLock);
        mProcess->mExecutingThreadsCount++;
        if (mProcess->mExecutingThreadsCount >= mProcess->mMaxThreads &&
                mProcess->mStarvationStartTimeMs == 0) {
            mProcess->mStarvationStartTimeMs = uptimeMillis();
        }
        pthread_mutex_unlock(&mProcess->mThreadCountLock);

        result = executeCommand(cmd);//执行binder命令

        pthread_mutex_lock(&mProcess->mThreadCountLock);
        mProcess->mExecutingThreadsCount--;
        if (mProcess->mExecutingThreadsCount < mProcess->mMaxThreads &&
                mProcess->mStarvationStartTimeMs != 0) {
            int64_t starvationTimeMs = uptimeMillis() - mProcess->mStarvationStartTimeMs;
            if (starvationTimeMs > 100) {
                ALOGE("binder thread pool (%zu threads) starved for %" PRId64 " ms",
                      mProcess->mMaxThreads, starvationTimeMs);
            }
            mProcess->mStarvationStartTimeMs = 0;
        }
        pthread_cond_broadcast(&mProcess->mThreadCountDecrement);
        pthread_mutex_unlock(&mProcess->mThreadCountLock);
    }

    return result;
}

void IPCThreadState::joinThreadPool(bool isMain)
{
    LOG_THREADPOOL("**** THREAD %p (PID %d) IS JOINING THE THREAD POOL\n", (void*)pthread_self(), getpid());

    mOut.writeInt32(isMain ? BC_ENTER_LOOPER : BC_REGISTER_LOOPER);

    status_t result;
    do {
        processPendingDerefs();
        // now get the next command to be processed, waiting if necessary
        result = getAndExecuteCommand(); //从binder驱动读取下一条数据并执行

        if (result < NO_ERROR && result != TIMED_OUT && result != -ECONNREFUSED && result != -EBADF) {
            ALOGE("getAndExecuteCommand(fd=%d) returned unexpected error %d, aborting",
                  mProcess->mDriverFD, result);
            abort();
        }

        // Let this thread exit the thread pool if it is no longer
        // needed and it is not the main process thread.
        if(result == TIMED_OUT && !isMain) {
            break;
        }
    } while (result != -ECONNREFUSED && result != -EBADF);

    LOG_THREADPOOL("**** THREAD %p (PID %d) IS LEAVING THE THREAD POOL err=%d\n",
        (void*)pthread_self(), getpid(), result);

    mOut.writeInt32(BC_EXIT_LOOPER);
    talkWithDriver(false);
}
```
在getAndExecuteCommand()执行之前会调用processPendingDerefs函数清除掉incoming command queue中处理完的内容，process any pending derefs。  
在processPendingDerefs函数中，当binder接收到的数据处理完时,清除mpendingweakderefs和mpendingstrongderefs vector向量中的内容。  

在IPCThread中有两个Vector分别为Vector<BBinder*>    mPendingStrongDerefs;和Vector<RefBase::weakref_type*> mPendingWeakDerefs;  
当在executeCommand中执行BR_DECREFS时会从mIn中读取要进行DECREFS减少弱引用计数操作的weakref对象并push到mPendingWeakDerefs Vector中。  
当在executeCommand中执行BR_RELEASE时会从mIn中读取要进行BR_RELEASE减少强引用计数操作的BBinder对象并push到mPendingStrongDerefs Vector中。  

```
status_t IPCThreadState::executeCommand(int32_t cmd)
{
    BBinder* obj;
    RefBase::weakref_type* refs;
    status_t result = NO_ERROR;

    switch ((uint32_t)cmd) {
    case BR_ERROR:
        result = mIn.readInt32();
        break;
......
    case BR_RELEASE:
        refs = (RefBase::weakref_type*)mIn.readPointer();
        obj = (BBinder*)mIn.readPointer();//读取BBinder对象
        ALOG_ASSERT(refs->refBase() == obj,
                   "BR_RELEASE: object %p does not match cookie %p (expected %p)",
                   refs, obj, refs->refBase());
        IF_LOG_REMOTEREFS() {
            LOG_REMOTEREFS("BR_RELEASE from driver on %p", obj);
            obj->printRefs();
        }
        mPendingStrongDerefs.push(obj);//push进mPendingStrongDerefs向量中。
        break;
......
    case BR_DECREFS:
        refs = (RefBase::weakref_type*)mIn.readPointer();//读取weakref对象
        obj = (BBinder*)mIn.readPointer();
        // NOTE: This assertion is not valid, because the object may no
        // longer exist (thus the (BBinder*)cast above resulting in a different
        // memory address).
        //ALOG_ASSERT(refs->refBase() == obj,
        //           "BR_DECREFS: object %p does not match cookie %p (expected %p)",
        //           refs, obj, refs->refBase());
        mPendingWeakDerefs.push(refs);//push进mPendingDerefs向量中。
        break;
......
    default:
        ALOGE("*** BAD COMMAND %d received from Binder driver\n", cmd);
        result = UNKNOWN_ERROR;
        break;
    }

    if (result != NO_ERROR) {
        mLastError = result;
    }

    return result;
}

//当binder接收到的数据处理完时,清除mpendingweakderefs和mpendingstrongderefs vector向量中的内容。
// When we've cleared the incoming command queue, process any pending derefs
void IPCThreadState::processPendingDerefs()
{
    if (mIn.dataPosition() >= mIn.dataSize()) {
        /*
         * The decWeak()/decStrong() calls may cause a destructor to run,
         * which in turn could have initiated an outgoing transaction,
         * which in turn could cause us to add to the pending refs
         * vectors; so instead of simply iterating, loop until they're empty.
         *
         * We do this in an outer loop, because calling decStrong()
         * may result in something being added to mPendingWeakDerefs,
         * which could be delayed until the next incoming command
         * from the driver if we don't process it now.
         */
        while (mPendingWeakDerefs.size() > 0 || mPendingStrongDerefs.size() > 0) {
            while (mPendingWeakDerefs.size() > 0) {
                RefBase::weakref_type* refs = mPendingWeakDerefs[0];
                mPendingWeakDerefs.removeAt(0);
                refs->decWeak(mProcess.get());
            }

            if (mPendingStrongDerefs.size() > 0) {
                // We don't use while() here because we don't want to re-order
                // strong and weak decs at all; if this decStrong() causes both a
                // decWeak() and a decStrong() to be queued, we want to process
                // the decWeak() first.
                BBinder* obj = mPendingStrongDerefs[0];
                mPendingStrongDerefs.removeAt(0);
                obj->decStrong(mProcess.get());
            }
        }
    }
}
```
## 4.Binder线程的任务
当接收Binder信息时Binder线程主要通过talkWithDriver函数，从binder驱动设备节点中读取数据，解析Command请求，并执行。  
```
status_t IPCThreadState::talkWithDriver(bool doReceive)
{
    if (mProcess->mDriverFD <= 0) {
        return -EBADF;
    }

    binder_write_read bwr;

    // Is the read buffer empty?
    const bool needRead = mIn.dataPosition() >= mIn.dataSize();

    // We don't want to write anything if we are still reading
    // from data left in the input buffer and the caller
    // has requested to read the next data.
    const size_t outAvail = (!doReceive || needRead) ? mOut.dataSize() : 0;

    bwr.write_size = outAvail;
    bwr.write_buffer = (uintptr_t)mOut.data();

    // This is what we'll read.
    if (doReceive && needRead) {
        bwr.read_size = mIn.dataCapacity();
        bwr.read_buffer = (uintptr_t)mIn.data();
    } else {
        bwr.read_size = 0;
        bwr.read_buffer = 0;
    }
......
    // Return immediately if there is nothing to do.
    if ((bwr.write_size == 0) && (bwr.read_size == 0)) return NO_ERROR;

    bwr.write_consumed = 0;
    bwr.read_consumed = 0;
    status_t err;
    do {
        IF_LOG_COMMANDS() {
            alog << "About to read/write, write size = " << mOut.dataSize() << endl;
        }
#if defined(__ANDROID__)
        if (ioctl(mProcess->mDriverFD, BINDER_WRITE_READ, &bwr) >= 0)
            err = NO_ERROR;
        else
            err = -errno;
#else
        err = INVALID_OPERATION;
#endif
        if (mProcess->mDriverFD <= 0) {
            err = -EBADF;
        }
        IF_LOG_COMMANDS() {
            alog << "Finished read/write, write size = " << mOut.dataSize() << endl;
        }
    } while (err == -EINTR);
......
    if (err >= NO_ERROR) {
        if (bwr.write_consumed > 0) {
            if (bwr.write_consumed < mOut.dataSize())
                mOut.remove(0, bwr.write_consumed);
            else {
                mOut.setDataSize(0);
                processPostWriteDerefs();
            }
        }
        if (bwr.read_consumed > 0) {
            mIn.setDataSize(bwr.read_consumed);
            mIn.setDataPosition(0);
        }
......
        }
        return NO_ERROR;
    }

    return err;
```
和binder驱动设备节点交互，读取数据通过ioctl的BINDER_WRITE_READ命令进行，数据结构形式为binder_write_read,其定义如下  
bionic/libc/kernel/uapi/linux/android/binder.h
```
struct binder_write_read {
  binder_size_t write_size;
  binder_size_t write_consumed;
  binder_uintptr_t write_buffer;
  binder_size_t read_size;
  binder_size_t read_consumed;
  binder_uintptr_t read_buffer;
};
```
从结构中我们看到该结构定义了大小，已经使用的大小以及数据位置的指针。
操作binder传输数据我们要通过Parcel对象进行，IPCThreadState中有Parcel mIn和Parcel mOut分别用来存储Read到的数据和要Write的数据。  
Parcel中的通过dataCapacity()获取到数据容量大小，对应binder_write_read中的read_size或write_size。  
Parcel中的data()函数获取数据的指针，对应binder_write_read中的read_buffer或write_buffer，当要通过ioctl从binder驱动读取数据时候，Parcel的data()函数获取的指针传递到binder_write_read中的read_buffer，ioctl操作后其被赋值为数据块的指针。  
Parcel的定义如下：  
include/binder/Parcel.h
```
class Parcel {
    friend class IPCThreadState;
public:
    class ReadableBlob;
    class WritableBlob;

                        Parcel();
                        ~Parcel();

    const uint8_t*      data() const;
    size_t              dataSize() const;
    size_t              dataAvail() const;
    size_t              dataPosition() const;
    size_t              dataCapacity() const;
......

    status_t            mError;
    uint8_t*            mData;
    size_t              mDataSize;
    size_t              mDataCapacity;
    mutable size_t      mDataPos;
    binder_size_t*      mObjects;
    size_t              mObjectsSize;
    size_t              mObjectsCapacity;
    mutable size_t      mNextObjectHint;
    mutable bool        mObjectsSorted;

    mutable bool        mFdsKnown;
    mutable bool        mHasFds;
    bool                mAllowFds;

    release_func        mOwner;
    void*               mOwnerCookie;

    class Blob {
    public:
        Blob();
        ~Blob();

        void clear();
        void release();
        inline size_t size() const { return mSize; }
        inline int fd() const { return mFd; }
        inline bool isMutable() const { return mMutable; }

    protected:
        void init(int fd, void* data, size_t size, bool isMutable);

        int mFd; // owned by parcel so not closed when released
        void* mData;
        size_t mSize;
        bool mMutable;
    };

    #if defined(__clang__)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wweak-vtables"
    #endif

    protected:
        ~FlattenableHelper() = default;
    public:
        virtual size_t getFlattenedSize() const {
            return val.getFlattenedSize();
        }
        virtual size_t getFdCount() const {
            return val.getFdCount();
        }
        virtual status_t flatten(void* buffer, size_t size, int* fds, size_t count) const {
            return val.flatten(buffer, size, fds, count);
        }
        virtual status_t unflatten(void const* buffer, size_t size, int const* fds, size_t count) {
            return const_cast<Flattenable<T>&>(val).unflatten(buffer, size, fds, count);
        }
    };
    status_t write(const FlattenableHelperInterface& val);
    status_t read(FlattenableHelperInterface& val) const;

public:
    class ReadableBlob : public Blob {
        friend class Parcel;
    public:
        inline const void* data() const { return mData; }
        inline void* mutableData() { return isMutable() ? mData : NULL; }
    };

    class WritableBlob : public Blob {
        friend class Parcel;
    public:
        inline void* data() { return mData; }
    };

private:
    size_t mOpenAshmemSize;

public:
    // TODO: Remove once ABI can be changed.
    size_t getBlobAshmemSize() const;
    size_t getOpenAshmemSize() const;
};
```
其内部定义了内部类class Blob和class ReadableBlob来进行Ashmem内存创建和访问。
## 5.binder传输协议
从上我们知道binder传输的数据通过Parcel打包到AshMem中，然后将指针传递给binder_write_read结构中的write_buffer中发送到binder驱动。  
或者从binder设备驱动读取binder_write_read结构，将其中的read_buffer指针传递给Parcel中。那么这个传递的数据里面有什么内容呢？  
其实传输的数据中包含命令和命令数据。  
从上面的介绍我们也知道对于传输的一次数据包含接收命令和发送命令。接收命令是BR开头的比如R_TRANSACTION_COMPLETE，BR_TRANSACTION等，而发送的命令以BC开头，比如BC_TRANSACTION,BC_REPLY等。比如A进程通过Binder和B进程通信，其发送BC命令给B进程，首先BC命令传递到binder驱动，binder驱动找到命令的目的端，然后发送相应的BR命令给B，并携带相应的数据。此时B进程就收到了A进程发送过来的BR命令。  
binder传输协议的BC命令和BR命令定义在bionic/libc/kernel/uapi/linux/android/binder.h中
```
enum binder_driver_return_protocol {
  BR_ERROR = _IOR('r', 0, __s32),
  BR_OK = _IO('r', 1),
  BR_TRANSACTION = _IOR('r', 2, struct binder_transaction_data),
  BR_REPLY = _IOR('r', 3, struct binder_transaction_data),
  BR_ACQUIRE_RESULT = _IOR('r', 4, __s32),
  BR_DEAD_REPLY = _IO('r', 5),
  BR_TRANSACTION_COMPLETE = _IO('r', 6),
  BR_INCREFS = _IOR('r', 7, struct binder_ptr_cookie),
  BR_ACQUIRE = _IOR('r', 8, struct binder_ptr_cookie),
  BR_RELEASE = _IOR('r', 9, struct binder_ptr_cookie),
  BR_DECREFS = _IOR('r', 10, struct binder_ptr_cookie),
  BR_ATTEMPT_ACQUIRE = _IOR('r', 11, struct binder_pri_ptr_cookie),
  BR_NOOP = _IO('r', 12),
  BR_SPAWN_LOOPER = _IO('r', 13),
  BR_FINISHED = _IO('r', 14),
  BR_DEAD_BINDER = _IOR('r', 15, binder_uintptr_t),
  BR_CLEAR_DEATH_NOTIFICATION_DONE = _IOR('r', 16, binder_uintptr_t),
  BR_FAILED_REPLY = _IO('r', 17),
};
enum binder_driver_command_protocol {
  BC_TRANSACTION = _IOW('c', 0, struct binder_transaction_data),
  BC_REPLY = _IOW('c', 1, struct binder_transaction_data),
  BC_ACQUIRE_RESULT = _IOW('c', 2, __s32),
  BC_FREE_BUFFER = _IOW('c', 3, binder_uintptr_t),
  BC_INCREFS = _IOW('c', 4, __u32),
  BC_ACQUIRE = _IOW('c', 5, __u32),
  BC_RELEASE = _IOW('c', 6, __u32),
  BC_DECREFS = _IOW('c', 7, __u32),
  BC_INCREFS_DONE = _IOW('c', 8, struct binder_ptr_cookie),
  BC_ACQUIRE_DONE = _IOW('c', 9, struct binder_ptr_cookie),
  BC_ATTEMPT_ACQUIRE = _IOW('c', 10, struct binder_pri_desc),
  BC_REGISTER_LOOPER = _IO('c', 11),
  BC_ENTER_LOOPER = _IO('c', 12),
  BC_EXIT_LOOPER = _IO('c', 13),
  BC_REQUEST_DEATH_NOTIFICATION = _IOW('c', 14, struct binder_handle_cookie),
  BC_CLEAR_DEATH_NOTIFICATION = _IOW('c', 15, struct binder_handle_cookie),
  BC_DEAD_BINDER_DONE = _IOW('c', 16, binder_uintptr_t),
  BC_TRANSACTION_SG = _IOW('c', 17, struct binder_transaction_data_sg),
  BC_REPLY_SG = _IOW('c', 18, struct binder_transaction_data_sg),
};
```

