---
title: binder传输
date: 2019-08-27 23:08:04
categories:
- 技术文章
tags:
- Android
- Binder
---

## 1.传输内容
我们通过transact函数向binder server侧发送数据.应用将数据打包到Parcel对象中,传递到IPCThreadState中，而writeTransactionData将数据封装到结构体struct binder_transaction_data中,并和命令BC_TRANSACTION一起打包到mOut中，这也是个Parcel对象， 
```
status_t IPCThreadState::transact(int32_t handle,
                                  uint32_t code, const Parcel& data,
                                  Parcel* reply, uint32_t flags)
{
    status_t err;
......
    LOG_ONEWAY(">>>> SEND from pid %d uid %d %s", getpid(), getuid(),
        (flags & TF_ONE_WAY) == 0 ? "READ REPLY" : "ONE WAY");
    err = writeTransactionData(BC_TRANSACTION, flags, handle, code, data, NULL);
......
}

status_t IPCThreadState::writeTransactionData(int32_t cmd, uint32_t binderFlags,
    int32_t handle, uint32_t code, const Parcel& data, status_t* statusBuffer)
{
   // 构建BC_TRANSACTION需要的数据类型。
    binder_transaction_data tr;

    tr.target.ptr = 0; /* Don't pass uninitialized stack data to a remote process */
    tr.target.handle = handle;
    tr.code = code;
    tr.flags = binderFlags;
    tr.cookie = 0;
    tr.sender_pid = 0;
    tr.sender_euid = 0;

    const status_t err = data.errorCheck();
    if (err == NO_ERROR) {
        tr.data_size = data.ipcDataSize();
        tr.data.ptr.buffer = data.ipcData();
        tr.offsets_size = data.ipcObjectsCount()*sizeof(binder_size_t);
        tr.data.ptr.offsets = data.ipcObjects();
    } else if (statusBuffer) {
        tr.flags |= TF_STATUS_CODE;
        *statusBuffer = err;
        tr.data_size = sizeof(status_t);
        tr.data.ptr.buffer = reinterpret_cast<uintptr_t>(statusBuffer);
        tr.offsets_size = 0;
        tr.data.ptr.offsets = 0;
    } else {
        return (mLastError = err);
    }

    // 构建一个binder数据命令块
    mOut.writeInt32(cmd);
    mOut.write(&tr, sizeof(tr));

    return NO_ERROR;
}
```

## 2.Parcel传输能的对象类型
Parcel中能存储各种类型，然后通过binder进行传输。
其中我们可以看到可以通过writeXXBinder将Binder对象存储到Parcel中用来传输。  
```
inline static status_t finish_flatten_binder(
    const sp<IBinder>& /*binder*/, const flat_binder_object& flat, Parcel* out)
{
    return out->writeObject(flat, false);
}

status_t flatten_binder(const sp<ProcessState>& /*proc*/,
    const wp<IBinder>& binder, Parcel* out)
{
    flat_binder_object obj;

    obj.flags = 0x7f | FLAT_BINDER_FLAG_ACCEPTS_FDS;
    if (binder != NULL) {
        sp<IBinder> real = binder.promote();
        if (real != NULL) {
            IBinder *local = real->localBinder();
            if (!local) {
                BpBinder *proxy = real->remoteBinder();
                if (proxy == NULL) {
                    ALOGE("null proxy");
                }
                const int32_t handle = proxy ? proxy->handle() : 0;
                obj.hdr.type = BINDER_TYPE_WEAK_HANDLE;
                obj.binder = 0; /* Don't pass uninitialized stack data to a remote process */
                obj.handle = handle;
                obj.cookie = 0;
            } else {
                obj.hdr.type = BINDER_TYPE_WEAK_BINDER;
                obj.binder = reinterpret_cast<uintptr_t>(binder.get_refs());
                obj.cookie = reinterpret_cast<uintptr_t>(binder.unsafe_get());
            }
            return finish_flatten_binder(real, obj, out);
        }

        // XXX How to deal?  In order to flatten the given binder,
        // we need to probe it for information, which requires a primary
        // reference...  but we don't have one.
        //
        // The OpenBinder implementation uses a dynamic_cast<> here,
        // but we can't do that with the different reference counting
        // implementation we are using.
        ALOGE("Unable to unflatten Binder weak reference!");
        obj.hdr.type = BINDER_TYPE_BINDER;
        obj.binder = 0;
        obj.cookie = 0;
        return finish_flatten_binder(NULL, obj, out);

    } else {
        obj.hdr.type = BINDER_TYPE_BINDER;
        obj.binder = 0;
        obj.cookie = 0;
        return finish_flatten_binder(NULL, obj, out);
    }
}

status_t Parcel::writeStrongBinder(const sp<IBinder>& val)
{
    return flatten_binder(ProcessState::self(), val, this);
}

status_t Parcel::writeStrongBinderVector(const std::vector<sp<IBinder>>& val)
{
    return writeTypedVector(val, &Parcel::writeStrongBinder);
}

status_t Parcel::writeStrongBinderVector(const std::unique_ptr<std::vector<sp<IBinder>>>& val)
{
    return writeNullableTypedVector(val, &Parcel::writeStrongBinder);
}

status_t Parcel::readStrongBinderVector(std::unique_ptr<std::vector<sp<IBinder>>>* val) const {
    return readNullableTypedVector(val, &Parcel::readNullableStrongBinder);
}

status_t Parcel::readStrongBinderVector(std::vector<sp<IBinder>>* val) const {
    return readTypedVector(val, &Parcel::readStrongBinder);
}

status_t Parcel::writeWeakBinder(const wp<IBinder>& val)
{
    return flatten_binder(ProcessState::self(), val, this);
}
```
