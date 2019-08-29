---
title: binder对象
date: 2019-08-28 16:08:04
categories:
- 技术文章
tags:
- Android
- Binder
---

## 1.查看进程中各类对象
我们一般通过"dumpsys meminfo 进程名"查看一个进程中各种对象的数目以及占用内存的大小。  
比如我们可以通过下面的命令查看systemui进程的内存占用情况。  
```
root@batman:~# adb shell dumpsys meminfo com.android.systemui
Applications Memory Usage (in Kilobytes):
Uptime: 32251338 Realtime: 357836066

** MEMINFO in pid 2425 [com.android.systemui] **
                   Pss  Private  Private  SwapPss     Heap     Heap     Heap
                 Total    Dirty    Clean    Dirty     Size    Alloc     Free
                ------   ------   ------   ------   ------   ------   ------
  Native Heap     8019     7996        0    53230    75776    65602    10173
  Dalvik Heap     5249     5236        0      583    10059     5030     5029
 Dalvik Other      936      936        0       72
        Stack       28       28        0       12
       Ashmem        2        0        0        0
      Gfx dev     3092     2948      144        0
    Other dev       36        0       36        0
     .so mmap     2961       84        8     1643
    .jar mmap        0        0        0      480
    .apk mmap     9902      176     7184     9752
    .ttf mmap      635        0      256        0
    .dex mmap     4638        0     3912        4
    .oat mmap     2212        0      208        0
    .art mmap     5091     4792       36      515
   Other mmap      189        0       64        4
   EGL mtrack    21380    21380        0        0
    GL mtrack     6560     6560        0        0
      Unknown      705      704        0      838
        TOTAL   138768    50840    11848    67133    85835    70632    15202

 App Summary
                       Pss(KB)
                        ------
           Java Heap:    10064
         Native Heap:     7996
                Code:    11828
               Stack:       28
            Graphics:    31032
       Private Other:     1740
              System:    76080

               TOTAL:   138768       TOTAL SWAP PSS:    67133

 Objects
               Views:     1249         ViewRootImpl:        2
         AppContexts:       30           Activities:        0
              Assets:       15        AssetManagers:        0
       Local Binders:      220        Proxy Binders:       87
       Parcel memory:       23         Parcel count:       95
    Death Recipients:        6      OpenSSL Sockets:        0
            WebViews:        0

 SQL
         MEMORY_USED:      320
  PAGECACHE_OVERFLOW:       38          MALLOC_SIZE:      117

 DATABASES
      pgsz     dbsz   Lookaside(b)          cache  Dbname
         4       16             29         3/16/2  /data/user_de/0/com.android.systemui/databases/desktopIcon.db
         4       20             29         1/16/2  /data/user_de/0/com.android.systemui/databases/fullscreenApp.db
```

关注Objects部分，我们可以看到systemui进程中和Binder相关的对象有如下几类  
- Loal Binders  
- Proxy Binders
- Parcel memory
- Parcle count
- Death Recipients
我们从代码入手看看这些对象都是什么。
frameworks/base/core/java/android/app/ActivityThread.java----->dumpMemInfo()
```
        private void dumpMemInfo(PrintWriter pw, Debug.MemoryInfo memInfo, boolean checkin,
                boolean dumpFullInfo, boolean dumpDalvik, boolean dumpSummaryOnly, boolean dumpUnreachable) {
            long nativeMax = Debug.getNativeHeapSize() / 1024;
            long nativeAllocated = Debug.getNativeHeapAllocatedSize() / 1024;
            long nativeFree = Debug.getNativeHeapFreeSize() / 1024;
            ......
            int binderLocalObjectCount = Debug.getBinderLocalObjectCount();
            int binderProxyObjectCount = Debug.getBinderProxyObjectCount();
            int binderDeathObjectCount = Debug.getBinderDeathObjectCount();
            long parcelSize = Parcel.getGlobalAllocSize();
            long parcelCount = Parcel.getGlobalAllocCount();
            ......

            printRow(pw, TWO_COUNT_COLUMNS, "Local Binders:", binderLocalObjectCount,
                    "Proxy Binders:", binderProxyObjectCount);
            printRow(pw, TWO_COUNT_COLUMNS, "Parcel memory:", parcelSize/1024,
                    "Parcel count:", parcelCount);
            printRow(pw, TWO_COUNT_COLUMNS, "Death Recipients:", binderDeathObjectCount,
                    "OpenSSL Sockets:", openSslSocketCount); 
            ......
```

从代码中我们知道进程中存在三种Binder对象Local Binders、Proxy Binders和Death Recipients，并且分别通过binderLocalObjectCount、binderProxyObjectCount和binderDeathObjectCount来记录其数目。  
获取这些计数的代码在frameworks/base/core/jni/android_util_Binder.cpp  
```
// Protected by gProxyLock. We warn if this gets too large.
static int32_t gNumProxies = 0;
static int32_t gProxiesWarned = 0;

// Number of GlobalRefs held by JavaBBinders.
static std::atomic<uint32_t> gNumLocalRefsCreated(0);
static std::atomic<uint32_t> gNumLocalRefsDeleted(0);
// Number of GlobalRefs held by JavaDeathRecipients.
static std::atomic<uint32_t> gNumDeathRefsCreated(0);
static std::atomic<uint32_t> gNumDeathRefsDeleted(0);
......
jint android_os_Debug_getLocalObjectCount(JNIEnv* env, jobject clazz)
{
    return gNumLocalRefsCreated - gNumLocalRefsDeleted;
}

jint android_os_Debug_getProxyObjectCount(JNIEnv* env, jobject clazz)
{
    AutoMutex _l(gProxyLock);
    return gNumProxies;
}

jint android_os_Debug_getDeathObjectCount(JNIEnv* env, jobject clazz)
{
    return gNumDeathRefsCreated - gNumDeathRefsDeleted;
}
```
我们首先看java层定义的Binder对象代码，其存在于frameworks/base/core/java/android/os/Binder.java   
- class Binder继承于IBinder
- class BinderProxy继承于IBinder
- class ProxyMap 通过key-value的形式存储管理BinderProxy

首先我们进入Binder的构造函数  
frameworks/base/core/java/android/os/Binder.java  
```
    /**
     * Default constructor initializes the object.
     */
    public Binder() {
        mObject = getNativeBBinderHolder();
        NoImagePreloadHolder.sRegistry.registerNativeAllocation(this, mObject);

        if (FIND_POTENTIAL_LEAKS) {
            final Class<? extends Binder> klass = getClass();
            if ((klass.isAnonymousClass() || klass.isMemberClass() || klass.isLocalClass()) &&
                    (klass.getModifiers() & Modifier.STATIC) == 0) {
                Log.w(TAG, "The following Binder class should be static or leaks might occur: " +
                    klass.getCanonicalName());
            }
        }
    }


    private static native long getNativeBBinderHolder();
    private static native long getFinalizer();
```
frameworks/base/core/jni/android_util_Binder.cpp  
```
static jlong android_os_Binder_getNativeBBinderHolder(JNIEnv* env, jobject clazz)
{
    JavaBBinderHolder* jbh = new JavaBBinderHolder();
    return (jlong) jbh;
}

class JavaBBinderHolder
{
public:
    sp<JavaBBinder> get(JNIEnv* env, jobject obj)
    {
        AutoMutex _l(mLock);
        sp<JavaBBinder> b = mBinder.promote();
        if (b == NULL) {
            b = new JavaBBinder(env, obj);
            mBinder = b;
            ALOGV("Creating JavaBinder %p (refs %p) for Object %p, weakCount=%" PRId32 "\n",
                 b.get(), b->getWeakRefs(), obj, b->getWeakRefs()->getWeakCount());
        }

        return b;
    }

    sp<JavaBBinder> getExisting()
    {
        AutoMutex _l(mLock);
        return mBinder.promote();
    }

private:
    Mutex           mLock;
    wp<JavaBBinder> mBinder;
};
```
可以看到Java层创建Binder对象时，会对应在native层创建一个JavaBBinder对象。  

native：  
JavaBBinder对象继承自BBinder对象

|JavaBBinder|
|:----:|
|onTransaction|

当JavaBBinder接收到Binder请求时将请求转发到Java层的处理处。
```
static int int_register_android_os_Binder(JNIEnv* env)
{
    jclass clazz = FindClassOrDie(env, kBinderPathName);

    gBinderOffsets.mClass = MakeGlobalRefOrDie(env, clazz);
    gBinderOffsets.mExecTransact = GetMethodIDOrDie(env, clazz, "execTransact", "(IJJI)Z"); // 获取java层的方法execTransact的methodID
    gBinderOffsets.mObject = GetFieldIDOrDie(env, clazz, "mObject", "J");

    return RegisterMethodsOrDie(
        env, kBinderPathName,
        gBinderMethods, NELEM(gBinderMethods));
}

class JavaBBinder : public BBinder
{
......
    virtual status_t onTransact(
        uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags = 0)
    {
        JNIEnv* env = javavm_to_jnienv(mVM);

        ALOGV("onTransact() on %p calling object %p in env %p vm %p\n", this, mObject, env, mVM);

        IPCThreadState* thread_state = IPCThreadState::self();
        const int32_t strict_policy_before = thread_state->getStrictModePolicy();

        //printf("Transact from %p to Java code sending: ", this);
        //data.print();
        //printf("\n");
        jboolean res = env->CallBooleanMethod(mObject, gBinderOffsets.mExecTransact,
            code, reinterpret_cast<jlong>(&data), reinterpret_cast<jlong>(reply), flags); //调用java层Binder的execTransact函数。

        if (env->ExceptionCheck()) {
            ScopedLocalRef<jthrowable> excep(env, env->ExceptionOccurred());
            report_exception(env, excep.get(),
                "*** Uncaught remote exception!  "
                "(Exceptions are not yet supported across processes.)");
            res = JNI_FALSE;
        }

        // Check if the strict mode state changed while processing the
        // call.  The Binder state will be restored by the underlying
        // Binder system in IPCThreadState, however we need to take care
        // of the parallel Java state as well.
        if (thread_state->getStrictModePolicy() != strict_policy_before) {
            set_dalvik_blockguard_policy(env, strict_policy_before);
        }

        if (env->ExceptionCheck()) {
            ScopedLocalRef<jthrowable> excep(env, env->ExceptionOccurred());
            report_exception(env, excep.get(),
                "*** Uncaught exception in onBinderStrictModePolicyChange");
        }

        // Need to always call through the native implementation of
        // SYSPROPS_TRANSACTION.
        if (code == SYSPROPS_TRANSACTION) {
            BBinder::onTransact(code, data, reply, flags);
        }

        //aout << "onTransact to Java code; result=" << res << endl
        //    << "Transact from " << this << " to Java code returning "
        //    << reply << ": " << *reply << endl;
        return res != JNI_FALSE ? NO_ERROR : UNKNOWN_TRANSACTION;
    }
......
}
```
这一切是通过下面的JNI方法调用JavaBinder的execTransact函数完成的  
```
jboolean res = env->CallBooleanMethod(mObject, gBinderOffsets.mExecTransact,
            code, reinterpret_cast<jlong>(&data), reinterpret_cast<jlong>(reply), flags);
```

frameworks/base/core/java/android/os/Binder.java
```
    // Entry point from android_util_Binder.cpp's onTransact
    private boolean execTransact(int code, long dataObj, long replyObj,
            int flags) {
        BinderCallsStats binderCallsStats = BinderCallsStats.getInstance();
        BinderCallsStats.CallSession callSession = binderCallsStats.callStarted(this, code);
        Parcel data = Parcel.obtain(dataObj);
        Parcel reply = Parcel.obtain(replyObj);
        // theoretically, we should call transact, which will call onTransact,
        // but all that does is rewind it, and we just got these from an IPC,
        // so we'll just call it directly.
        boolean res;
        // Log any exceptions as warnings, don't silently suppress them.
        // If the call was FLAG_ONEWAY then these exceptions disappear into the ether.
        final boolean tracingEnabled = Binder.isTracingEnabled();
        try {
            if (tracingEnabled) {
                Trace.traceBegin(Trace.TRACE_TAG_ALWAYS, getClass().getName() + ":" + code);
            }
            res = onTransact(code, data, reply, flags); //运行onTransaction
        } catch (RemoteException|RuntimeException e) {
            if (LOG_RUNTIME_EXCEPTION) {
                Log.w(TAG, "Caught a RuntimeException from the binder stub implementation.", e);
            }
            if ((flags & FLAG_ONEWAY) != 0) {
                if (e instanceof RemoteException) {
                    Log.w(TAG, "Binder call failed.", e);
                } else {
                    Log.w(TAG, "Caught a RuntimeException from the binder stub implementation.", e);
                }
            } else {
                reply.setDataPosition(0);
                reply.writeException(e);
            }
            res = true;
        } finally {
            if (tracingEnabled) {
                Trace.traceEnd(Trace.TRACE_TAG_ALWAYS);
            }
        }
        checkParcel(this, code, reply, "Unreasonably large binder reply buffer");
        reply.recycle();
        data.recycle();

        // Just in case -- we are done with the IPC, so there should be no more strict
        // mode violations that have gathered for this thread.  Either they have been
        // parceled and are now in transport off to the caller, or we are returning back
        // to the main transaction loop to wait for another incoming transaction.  Either
        // way, strict mode begone!
        StrictMode.clearGatheredViolations();
        binderCallsStats.callEnded(callSession);

        return res;
    }
}
```

## 2.创建BBinder的本质


## 3.Proxy Binders
增加gNumProxies数目的代码在函数javaObjectForIBinder中
```
// If the argument is a JavaBBinder, return the Java object that was used to create it.
// Otherwise return a BinderProxy for the IBinder. If a previous call was passed the
// same IBinder, and the original BinderProxy is still alive, return the same BinderProxy.
jobject javaObjectForIBinder(JNIEnv* env, const sp<IBinder>& val)
{
    if (val == NULL) return NULL;

    if (val->checkSubclass(&gBinderOffsets)) {
        // It's a JavaBBinder created by ibinderForJavaObject. Already has Java object.
        jobject object = static_cast<JavaBBinder*>(val.get())->object();
        LOGDEATH("objectForBinder %p: it's our own %p!\n", val.get(), object);
        return object;
    }

    // For the rest of the function we will hold this lock, to serialize
    // looking/creation/destruction of Java proxies for native Binder proxies.
    AutoMutex _l(gProxyLock);

    BinderProxyNativeData* nativeData = gNativeDataCache;
    if (nativeData == nullptr) {
        nativeData = new BinderProxyNativeData();
    }
    // gNativeDataCache is now logically empty.
    jobject object = env->CallStaticObjectMethod(gBinderProxyOffsets.mClass,
            gBinderProxyOffsets.mGetInstance, (jlong) nativeData, (jlong) val.get());
    if (env->ExceptionCheck()) {
        // In the exception case, getInstance still took ownership of nativeData.
        gNativeDataCache = nullptr;
        return NULL;
    }
    BinderProxyNativeData* actualNativeData = getBPNativeData(env, object);
    if (actualNativeData == nativeData) {
        // New BinderProxy; we still have exclusive access.
        nativeData->mOrgue = new DeathRecipientList;
        nativeData->mObject = val;
        gNativeDataCache = nullptr;
        ++gNumProxies;
        if (gNumProxies >= gProxiesWarned + PROXY_WARN_INTERVAL) {
            ALOGW("Unexpectedly many live BinderProxies: %d\n", gNumProxies);
            gProxiesWarned = gNumProxies;
        }
    } else {
        // nativeData wasn't used. Reuse it the next time.
        gNativeDataCache = nativeData;
    }

    return object;
}
```

## 3.Native Binder对象
native层的binder对象有Server侧的BBinder和Client侧的BpBinder, 他们的通信流程如下图。  
![img](/images/binder对象/BpBinder_BBinder.png)
Client可以通过创建BpBinder对象构造和特定Server侧通信的Client Binder对象，其中最重要的是handle，每个handle对应一个Server的BB对象。  
```
class BpBinder : public IBinder
{
public:
    static BpBinder*    create(int32_t handle);
......
protected:
                        BpBinder(int32_t handle,int32_t trackedUid);
    virtual             ~BpBinder();
......
};
```
系统中的servicemanager进程中启动的BBinder对应的handle是0。我们可以通过使用下面的代码构建servicemanager的binder Client侧，然后和其通信。  
```
  sp<IBinder> servicemanager(BpBinder.create(0));
```
defaultServiceManager就是通过这种方式获取的。  
"libs/binder/IServiceManager.cpp"
```
sp<IServiceManager> defaultServiceManager()
{
    if (gDefaultServiceManager != NULL) return gDefaultServiceManager;

    {
        AutoMutex _l(gDefaultServiceManagerLock);
        while (gDefaultServiceManager == NULL) {
            gDefaultServiceManager = interface_cast<IServiceManager>(
                ProcessState::self()->getContextObject(NULL));
            if (gDefaultServiceManager == NULL)
                sleep(1);
        }
    }

    return gDefaultServiceManager;
}
```
此处getContextObject()走的是下面的代码,也是创建了一个handle为0的BpBinder。
"libs/binder/ProcessState.cpp"
```
sp<IBinder> ProcessState::getContextObject(const sp<IBinder>& /*caller*/)
{
    return getStrongProxyForHandle(0);
}

sp<IBinder> ProcessState::getStrongProxyForHandle(int32_t handle)
{
    sp<IBinder> result;

    AutoMutex _l(mLock);

    handle_entry* e = lookupHandleLocked(handle);

    if (e != NULL) {
......
            b = BpBinder::create(handle);
            e->binder = b;
            if (b) e->refs = b->getWeakRefs();
            result = b;
        } else {
......
}
```

在ProcessState中还有一个getContextObject用来获取IBinder对象，代码如下：  
```
sp<IBinder> ProcessState::getContextObject(const String16& name, const sp<IBinder>& caller)
{
    mLock.lock();
    sp<IBinder> object(
        mContexts.indexOfKey(name) >= 0 ? mContexts.valueFor(name) : NULL);
    mLock.unlock();

    //printf("Getting context object %s for %p\n", String8(name).string(), caller.get());

    if (object != NULL) return object;

    // Don't attempt to retrieve contexts if we manage them
    if (mManagesContexts) {
        ALOGE("getContextObject(%s) failed, but we manage the contexts!\n",
            String8(name).string());
        return NULL;
    }

    IPCThreadState* ipc = IPCThreadState::self();
    {
        Parcel data, reply;
        // no interface token on this magic transaction
        data.writeString16(name);
        data.writeStrongBinder(caller);
        status_t result = ipc->transact(0 /*magic*/, 0, data, &reply, 0);
        if (result == NO_ERROR) {
            object = reply.readStrongBinder();
        }
    }

    ipc->flushCommands();

    if (object != NULL) setContextObject(object, name);
    return object;
}
```
