---
title: Android Log系统
categories:
  - 技术文章
date: 2019-09-10 09:52:41
tags:
  - Android
---

https://blog.csdn.net/rikeyone/article/details/80307975
https://elinux.org/Android_Logging_System

## 概要
#### Android 8.0(O)之前：
Android的log包含两部分，内核中产生的，用户空间产生的。  
内核空间产生的log通过Linux内核中的log系统进行处理，可以通过dmesg和/proc/kmsg进行访问。  
用户空间的log系统我们称Logging系统将产生的log缓存到内核buffers中，整体架构如下：  
![img](/images/AndroidLogSystem/Overview.png)
Logging系统包含如下部分：  
- 一个字符设备驱动以及内核中用来存储log内容的buffers区。  
- 用来向Logging系统输入Log，以及查看Log的C、C++以及Java库。  
- 一个用来查看log信息的程序-logcat。  

Linux内核中有4个不同的buffers区，分别用来缓存不同类别的log。不同类别的log通过/dev/log目录下的不同设备节点进行访问。  
- main:the main application log
- events:for system event information 
- radio:for radio and phone-related information
- system:a log for low-level system messages and debugging 


#### Android 9.0(P)之后：
Android 的Logging系统在用户空间构建，logd进程负责收集缓存log，liblog通过socket负责和logd进程通信。  
logd通过系统属性暴露接口，控制logd的属性。  

|name                       | type |default  |description|
|:----                      |:----: | ----  | ---- |
|ro.logd.auditd             |bool   |true    |Enable selinux audit daemon|
|ro.logd.auditd.dmesg       |bool   |true    |selinux audit messages sent to dmesg.
|ro.logd.auditd.main        |bool   |true    |selinux audit messages sent to main.
|ro.logd.auditd.events      |bool   |true    |selinux audit messages sent to events.
|persist.logd.security      |bool   |false   |Enable security buffer.
|ro.device_owner            |bool   |false   |Override persist.logd.security to false
|ro.logd.kernel             |bool+  |svelte+ |Enable klogd daemon
|ro.logd.statistics         |bool+  |svelte+ |Enable logcat -S statistics.
|ro.debuggable              |number |        |if not "1", logd.statistics & ro.logd.kernel default false.
|logd.logpersistd.enable    |bool   |auto    |Safe to start logpersist daemon service
|logd.logpersistd           |string |persist |Enable logpersist daemon, "logcatd" turns on logcat -f in logd context. Responds to logcatd, clear and stop.
|logd.logpersistd.buffer    |       |persist |logpersistd buffers to collect
|logd.logpersistd.size      |       |persist |logpersistd size in MB
|persist.logd.logpersistd   |string |        |Enable logpersist daemon, "logcatd" turns on logcat -f in logd context.
|persist.logd.logpersistd.buffer |  |all     |logpersistd buffers to collect
|persist.logd.logpersistd.size   |  |256     |logpersistd size in MB
|persist.logd.size          |number |ro      |Global default size of the buffer for all log ids at initial startup, at runtime use: logcat -b all -G <value>
|ro.logd.size               |number |svelte  |default for persist.logd.size. Larger platform default sizes than 256KB are known to not scale well under log spam pressure. Address the spam first, resist increasing the log buffer.
|persist.logd.size.<buffer> |number |ro      |Size of the buffer for <buffer> log
|ro.logd.size.<buffer>      |number |svelte  |default for persist.logd.size.<buffer>
|ro.config.low_ram          |bool   |false   |if true, logd.statistics, ro.logd.kernel default false, logd.size 64K instead of 256K.
|persist.logd.filter        |string |        |Pruning filter to optimize content. At runtime use: logcat -P "<string>"
|ro.logd.filter             |string |"~! ~1000/!"| default for persist.logd.filter.This default means to prune the oldest entries of chattiest UID, and the chattiest PID of system (1000, or AID_SYSTEM).
|persist.logd.timestamp     |string |  ro    |The recording timestamp source. "m[onotonic]" is the only supported key character, otherwise realtime.
|ro.logd.timestamp          |string |        |realtime default for persist.logd.timestamp
|log.tag                    |string |        |persist The global logging level, VERBOSE,DEBUG, INFO, WARN, ERROR, ASSERT or SILENT. Only the first character is the key character.
|persist.log.tag            |string |        |build  default for log.tag
|log.tag.<tag>              |string |        |persist The <tag> specific logging level.
|persist.log.tag.<tag>      |string |        |build  default for log.tag.<tag>

logd创建了3个socket通道用于通信分别是：
- /dev/socket/logd
- /dev/socket/logdr
- /dev/socket/logdw
1.CommandListener-logd  
![img](/images/AndroidLogSystem/CommandListenerUML.png)
 整体流程大概如下：  
  首先CommandListener构建时打开设备节点/dev/socket/logd,并监听Client的请求，同时通过registerCmd设置能够处理的请求。  
  registerCmd的操作是将Server能处理的请求加入到mCommands队列中。
  当接收到Client请求时，构造一个SocketClient并加入到队列mClients中。
  同时CommandListener对Client发送来的Command请求，到mCommands队列中去查找，当查找到相应的FrameworkCommnad时，使用其runCommand函数执行请求。
  在CommandListener的构造函数中可以看到其设置并可以处理的Command。  
  ```cpp
  CommandListener::CommandListener(LogBuffer* buf, LogReader* /*reader*/,
                                 LogListener* /*swl*/)
    : FrameworkListener(getLogSocket()) {
    // registerCmd(new ShutdownCmd(buf, writer, swl));
    registerCmd(new ClearCmd(buf));
    registerCmd(new GetBufSizeCmd(buf));
    registerCmd(new SetBufSizeCmd(buf));
    registerCmd(new GetBufSizeUsedCmd(buf));
    registerCmd(new GetStatisticsCmd(buf));
    registerCmd(new SetPruneListCmd(buf));
    registerCmd(new GetPruneListCmd(buf));
    registerCmd(new GetEventTagCmd(buf));
    registerCmd(new ReinitCmd());
    registerCmd(new ExitCmd(this));
  }
  ```
2.log缓存Buffer  
logd中通过LogBuffer缓存log消息。LogBuffer结构如下：  
![img](/images/AndroidLogSystem/LogBufferUML.png)
所有发送到logd的log信息都构建成了LogBufferElement元素存储在列表mLogElements中。  
logd/LogBuffer.cpp  
```cpp
// assumes LogBuffer::wrlock() held, owns elem, look after garbage collection
void LogBuffer::log(LogBufferElement* elem) {
    // cap on how far back we will sort in-place, otherwise append
    static uint32_t too_far_back = 5;  // five seconds
    // Insert elements in time sorted order if possible
    //  NB: if end is region locked, place element at end of list
    LogBufferElementCollection::iterator it = mLogElements.end();
    LogBufferElementCollection::iterator last = it;
    if (__predict_true(it != mLogElements.begin())) --it;
    if (__predict_false(it == mLogElements.begin()) ||
        __predict_true((*it)->getRealTime() <= elem->getRealTime()) ||
        __predict_false((((*it)->getRealTime().tv_sec - too_far_back) >
                         elem->getRealTime().tv_sec) &&
                        (elem->getLogId() != LOG_ID_KERNEL) &&
                        ((*it)->getLogId() != LOG_ID_KERNEL))) {
        mLogElements.push_back(elem);                                //将LogBufferElement元素存入到mLogElemnts中
    } else {
......
        if (end_always || (end_set && (end > (*it)->getRealTime()))) {
            mLogElements.push_back(elem);                            //将LogBufferElement元素存入到mLogElements中
        } else {
            // should be short as timestamps are localized near end()
            do {
                last = it;
                if (__predict_false(it == mLogElements.begin())) {
                    break;
                }
                --it;
            } while (((*it)->getRealTime() > elem->getRealTime()) &&
                     (!end_set || (end <= (*it)->getRealTime())));
            mLogElements.insert(last, elem);                        //将LogBufferElement元素存入到mLogElements中
        }
        LogTimeEntry::unlock();
    }

    stats.add(elem);
    maybePrune(elem->getLogId());
}
```

3.从logd读取log 
客户端如logcat通过/dev/sock/logdr socket文件和logd交互读取log，logd读取侧的结构如下：  
![img](/images/AndroidLogSystem/LogReaderUML.png)
LogReader构建时开始监听socket /dev/sock/logdr的数据，当接收到读取log的请求时，调用onDataAvailable()函数处理请求。  
在onDataAvailable()函数中会构造FlushCommand对象并通过runSocketCommand()函数向Client发送其需要的log。  
而在runSocketCommand中其构造了LogTimeEntry，并用startReader_Locked()函数启动了一个线程，将LogBuffer中的数据发送到Client中。  
在这之后其他的Client能再次通过/dev/sock/logdr请求log数据。  
logd/LogTimes.cpp
```cpp
void* LogTimeEntry::threadStart(void* obj) {
    prctl(PR_SET_NAME, "logd.reader.per");

    LogTimeEntry* me = reinterpret_cast<LogTimeEntry*>(obj);

    pthread_cleanup_push(threadStop, obj);

    SocketClient* client = me->mClient;
    if (!client) {
        me->error();
        return nullptr;
    }

    LogBuffer& logbuf = me->mReader.logbuf();

    bool privileged = FlushCommand::hasReadLogs(client);
    bool security = FlushCommand::hasSecurityLogs(client);

    me->leadingDropped = true;

    wrlock();

    log_time start = me->mStart;

    while (me->threadRunning && !me->isError_Locked()) {
        if (me->mTimeout.tv_sec || me->mTimeout.tv_nsec) {
            if (pthread_cond_timedwait(&me->threadTriggeredCondition,
                                       &timesLock, &me->mTimeout) == ETIMEDOUT) {
                me->mTimeout.tv_sec = 0;
                me->mTimeout.tv_nsec = 0;
            }
            if (!me->threadRunning || me->isError_Locked()) {
                break;
            }
        }

        unlock();

        if (me->mTail) {
            logbuf.flushTo(client, start, nullptr, privileged, security,
                           FilterFirstPass, me);
            me->leadingDropped = true;
        }
        start = logbuf.flushTo(client, start, me->mLastTid, privileged,
                               security, FilterSecondPass, me);

        wrlock();

        if (start == LogBufferElement::FLUSH_ERROR) {
            me->error_Locked();
            break;
        }

        me->mStart = start + log_time(0, 1);

        if (me->mNonBlock || !me->threadRunning || me->isError_Locked()) {
            break;
        }

        me->cleanSkip_Locked();

        if (!me->mTimeout.tv_sec && !me->mTimeout.tv_nsec) {
            pthread_cond_wait(&me->threadTriggeredCondition, &timesLock);
        }
    }

    unlock();

    pthread_cleanup_pop(true);

    return nullptr;
}
```

4.log写入logd  
在logd侧写入log比较简单，logd中接收Client传递过来的log数据后执行onDataAvilable()，并将其写入到LogBuffer中。  
![img](/images/AndroidLogSystem/LogListenerUML.png)

## liblog
#### Android 8.0(O)之前

#### Android 9.0(P)之后
  


## 标准输出、标准错误
在Android有一些应用程序的日志输出是通过printf之类的标准函数输出的，这类log是无法记录到Logging系统的。
主要是由于init进程会把0,1,2三个fd指向到/dev/null，而其他进程都是由init fork出来的，所以标准输出、标准错误输出都会继承自父进程，默认也都是不打印出来的。
main.cpp(main)->init.cpp(main)->log.cpp(InitKernelLogging)   
system/core/init/log.cpp  
```cpp
void InitKernelLogging(char* argv[]) {
    // Make stdin/stdout/stderr all point to /dev/null.
    int fd = open("/sys/fs/selinux/null", O_RDWR);
    if (fd == -1) {
        int saved_errno = errno;
        android::base::InitLogging(argv, &android::base::KernelLogger, InitAborter);
        errno = saved_errno;
        PLOG(FATAL) << "Couldn't open /sys/fs/selinux/null";
    }
    dup2(fd, 0);
    dup2(fd, 1);
    dup2(fd, 2);
    if (fd > 2) close(fd);

    android::base::InitLogging(argv, &android::base::KernelLogger, InitAborter);
}
```

## 标准输出、标准错误log输出
android中提供了logwrapper程序用来重定向标准输出、标准错误的log输出，重定向的log可以使用logcat查看.  
源代码实现在system/core/logwrapper中。  
使用logwrapper命令来执行第三方应用程序，logwrapper命令可以把第三方应用程序的标准输出重定向到logcat的日志系统中去（缺省级别为LOG_INFO，标签为应用程序名）。  
```
PD1829:/ # logwrapper -h
logwrapper: invalid option -- h
Usage: logwrapper [-a] [-d] [-k] BINARY [ARGS ...]

Forks and executes BINARY ARGS, redirecting stdout and stderr to
the Android logging system. Tag is set to BINARY, priority is
always LOG_INFO.

-a: Causes logwrapper to do abbreviated logging.
    This logs up to the first 4K and last 4K of the command
    being run, and logs the output when the command exits
-d: Causes logwrapper to SIGSEGV when BINARY terminates
    fault address is set to the status of wait()
-k: Causes logwrapper to log to the kernel log instead of
    the Android system log
```
我们写一个程序通过printf向标准输出打印helloworld，直接执行时无法看到adb logcat中有相应的log。
```cpp
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char* const argv[])
{
    while (1) {
        void *p = malloc(100);
        printf("helloworld from testprintf\n");
        sleep(10);
        free(p);
    }
}
```
当我们通过logwrapper对log进行重定向时就能通过adb logcat看到输出的hellworld了。  
```bash
130|PD1829:/ # logwrapper testprintf
```
```
09-11 12:07:06.375 I/testprintf(12056): helloworld from testprintf
```


