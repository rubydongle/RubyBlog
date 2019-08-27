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

