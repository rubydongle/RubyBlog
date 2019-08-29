---
title: ART内存分配器之LinearAlloc
categories:
  - 技术文章
date: 2019-08-29 14:40:44
tags:
  - Android
  - Art
---
## 1.内存分配区
Android Runtime有两种虚拟机运行时，Dalvik runtime和ART runtime，它们分配内存的内存区域是不同的：
- Dalvik  
    Linear Alloc内存区
    Zygote Space
    Alloc Space

- ART  
    Non Moving Space
    Zygote Space
    Alloc Space
    Image Space
    Large Object Space

不管是Dalvik和ART，Android runtime中内存区的划分都有LinearAlloc(相当于ART的Non Moving Space)、Zygote Space、和Alloc Space.  
- Linear Alloc内存区是一个只读的线性内存空间，主要用来存储虚拟机中的类对象。  
  Linear Alloc区中的类对象加载后相当于执行的静态代码只需要读取而不会改变。将这些是只读属性，并且在整个进程生命周期都不会结束清理的永久数据放到线性分配的内存区间中管理有很多好处，能很好地减少堆混乱和GC扫描，提升内存管理的性能。  
- Zygote Space是一个可以在Zygote进程和应用程序进程之间进行共享的内存区间。
    由于Android中的所有进程都是由fork系统调用从Zygote进程fork过来的，所以他们有部分内容是一样的。这部分内容放在Zygote Space内存区，供Zygote和其子进程共享。 
- Allocation Space则是每个进程独占的，用来分配对象的内存的空间。

*Zygote Space和Allocation Space的关键比较不一般。*   
Android的第一个虚拟机是由Zygote进程创建的。Zygote的创建过程中创建了Zygote Space，此时Zygote Space和Alloc Space还没分开。  
当Zygote进程在fork第一个子进程之前，会去将Zygote一分为二。  
一部分已经使用的内存区为Zygote Space，它在整个系统中是唯一的一块内存区，并且和Zygote fork出来的子进程是共享可读的。  
另一部分没有使用的Zygote Space变为Zygote进程的Allocation Space。  
后面无论是应用进程还是Zygote进程，当他们要分配对象时，都在Allocation上进行的。  

在ART runtime中还有另外两个内存区，Image Space和Large Object Space。  
- Image Space
    存放一些预加载类，类似于Dalvik中的Linear Alloc。与Zygote Space一样，在Zygote进程和应用程序进程之间共享。
    Image Space的对象只创建一次，而Zygote Space的对象需要在系统每次启动时，根据运行情况都重新创建一遍。
- Large Object Space
    离散地址的集合，分配一些大对象，用于提高GC的管理效率和整体性能。

## 2.Linear Alloc内存区内存分配器的创建
Linear Alloc内存分配器的申明和实现在代码art/runtime/linear_alloc.h和art/runtime/linear_alloc.cpp  
我们可以通过该分配器从Linear Alloc内存区分配Linear Alloc内存区的内存。  
```
class LinearAlloc {
 public:
  explicit LinearAlloc(ArenaPool* pool);

  void* Alloc(Thread* self, size_t size) REQUIRES(!lock_);
  void* AllocAlign16(Thread* self, size_t size) REQUIRES(!lock_);

  // Realloc never frees the input pointer, it is the caller's job to do this if necessary.
  void* Realloc(Thread* self, void* ptr, size_t old_size, size_t new_size) REQUIRES(!lock_);

  // Allocate an array of structs of type T.
  template<class T>
  T* AllocArray(Thread* self, size_t elements) REQUIRES(!lock_) {
    return reinterpret_cast<T*>(Alloc(self, elements * sizeof(T)));
  }

  // Return the number of bytes used in the allocator.
  size_t GetUsedMemory() const REQUIRES(!lock_);

  ArenaPool* GetArenaPool() REQUIRES(!lock_);

  // Return true if the linear alloc contrains an address.
  bool Contains(void* ptr) const REQUIRES(!lock_);

  // Unsafe version of 'Contains' only to be used when the allocator is going
  // to be deleted.
  bool ContainsUnsafe(void* ptr) const NO_THREAD_SAFETY_ANALYSIS;

 private:
  mutable Mutex lock_ DEFAULT_MUTEX_ACQUIRED_AFTER;
  ArenaAllocator allocator_ GUARDED_BY(lock_);

  DISALLOW_IMPLICIT_CONSTRUCTORS(LinearAlloc);
};
```
art 在runtime的Init函数中创建Linear Alloc内存区分配器的。  
```
bool Runtime::Init(RuntimeArgumentMap&& runtime_options_in) {
  // (b/30160149): protect subprocesses from modifications to LD_LIBRARY_PATH, etc.
  // Take a snapshot of the environment at the time the runtime was created, for use by Exec, etc.
  env_snapshot_.TakeSnapshot();
......
  // Use MemMap arena pool for jit, malloc otherwise. Malloc arenas are faster to allocate but
  // can't be trimmed as easily.
  const bool use_malloc = IsAotCompiler();
  arena_pool_.reset(new ArenaPool(use_malloc, /* low_4gb */ false));
  jit_arena_pool_.reset(
      new ArenaPool(/* use_malloc */ false, /* low_4gb */ false, "CompilerMetadata"));

  if (IsAotCompiler() && Is64BitInstructionSet(kRuntimeISA)) {
    // 4gb, no malloc. Explanation in header.
    low_4gb_arena_pool_.reset(new ArenaPool(/* use_malloc */ false, /* low_4gb */ true));
  }
  linear_alloc_.reset(CreateLinearAlloc());
......
}

LinearAlloc* Runtime::CreateLinearAlloc() {
  // For 64 bit compilers, it needs to be in low 4GB in the case where we are cross compiling for a
  // 32 bit target. In this case, we have 32 bit pointers in the dex cache arrays which can't hold
  // when we have 64 bit ArtMethod pointers.
  return (IsAotCompiler() && Is64BitInstructionSet(kRuntimeISA))
      ? new LinearAlloc(low_4gb_arena_pool_.get())
      : new LinearAlloc(arena_pool_.get());
}
```
我们可以看到该内存分配器的创建依赖ArenaAllocator分配器
```
ArenaAllocator allocator_ GUARDED_BY(lock_);

```
而ArenaAlloc内存分配器是通过ArenaPool构建的。
```
class ArenaAllocator
    : private DebugStackRefCounter, private ArenaAllocatorStats, private ArenaAllocatorMemoryTool {
 public:
  explicit ArenaAllocator(ArenaPool* pool);
  ~ArenaAllocator();
......
```
我们通过创建的ArenaPool内存池创建LinearAlloc内存分配器，也就是相当于我们从ArenaPool内存池中获取一块内存，这块内存作为Linear Alloc内存区。  

### 3.内存池中获取Linear Alloc内存区
LinearAlloc内存分配器分配内存的代码如下：
```
void* LinearAlloc::Alloc(Thread* self, size_t size) {
  MutexLock mu(self, lock_);
  return allocator_.Alloc(size);
}
```
从上面的分析我们可以知道它是使用ArenaAllocator内存分配器分配内存的
```
// Fast single-threaded allocator for zero-initialized memory chunks.
//
// Memory is allocated from ArenaPool in large chunks and then rationed through
// the ArenaAllocator. It's returned to the ArenaPool only when the ArenaAllocator
// is destroyed.
class ArenaAllocator
    : private DebugStackRefCounter, private ArenaAllocatorStats, private ArenaAllocatorMemoryTool {
 public:
  explicit ArenaAllocator(ArenaPool* pool);
  ~ArenaAllocator();

  using ArenaAllocatorMemoryTool::IsRunningOnMemoryTool;
  using ArenaAllocatorMemoryTool::MakeDefined;
  using ArenaAllocatorMemoryTool::MakeUndefined;
  using ArenaAllocatorMemoryTool::MakeInaccessible;

  // Get adapter for use in STL containers. See arena_containers.h .
  ArenaAllocatorAdapter<void> Adapter(ArenaAllocKind kind = kArenaAllocSTL);

  // Returns zeroed memory.
  void* Alloc(size_t bytes, ArenaAllocKind kind = kArenaAllocMisc) ALWAYS_INLINE {
    if (UNLIKELY(IsRunningOnMemoryTool())) {
      return AllocWithMemoryTool(bytes, kind);
    }
    bytes = RoundUp(bytes, kAlignment);
    ArenaAllocatorStats::RecordAlloc(bytes, kind);
    if (UNLIKELY(bytes > static_cast<size_t>(end_ - ptr_))) {
      return AllocFromNewArena(bytes);
    }
    uint8_t* ret = ptr_;
    DCHECK_ALIGNED(ret, kAlignment);
    ptr_ += bytes;
    return ret;
  }

uint8_t* ArenaAllocator::AllocFromNewArena(size_t bytes) {
  Arena* new_arena = pool_->AllocArena(std::max(arena_allocator::kArenaDefaultSize, bytes));
  DCHECK(new_arena != nullptr);
  DCHECK_LE(bytes, new_arena->Size());
  if (static_cast<size_t>(end_ - ptr_) > new_arena->Size() - bytes) {
    // The old arena has more space remaining than the new one, so keep using it.
    // This can happen when the requested size is over half of the default size.
    DCHECK(arena_head_ != nullptr);
    new_arena->bytes_allocated_ = bytes;  // UpdateBytesAllocated() on the new_arena.
    new_arena->next_ = arena_head_->next_;
    arena_head_->next_ = new_arena;
  } else {
    UpdateBytesAllocated();
    new_arena->next_ = arena_head_;
    arena_head_ = new_arena;
    // Update our internal data structures.
    begin_ = new_arena->Begin();
    DCHECK_ALIGNED(begin_, kAlignment);
    ptr_ = begin_ + bytes;
    end_ = new_arena->End();
  }
  return new_arena->Begin();
}
```
从上我们看到new_arena是从AneraPool内存池中获取的。  
```
Arena* ArenaPool::AllocArena(size_t size) {
  Thread* self = Thread::Current();
  Arena* ret = nullptr;
  {
    MutexLock lock(self, lock_);
    if (free_arenas_ != nullptr && LIKELY(free_arenas_->Size() >= size)) {
      ret = free_arenas_;
      free_arenas_ = free_arenas_->next_;
    }
  }
  if (ret == nullptr) {
    ret = use_malloc_ ? static_cast<Arena*>(new MallocArena(size)) :
        new MemMapArena(size, low_4gb_, name_);
  }
  ret->Reset();
  return ret;
}
```
那么这个操作 new MemMapArena是我们想要的内容。
```
MemMapArena::MemMapArena(size_t size, bool low_4gb, const char* name) {
  // Round up to a full page as that's the smallest unit of allocation for mmap()
  // and we want to be able to use all memory that we actually allocate.
  size = RoundUp(size, kPageSize);
  std::string error_msg;
  map_.reset(MemMap::MapAnonymous(
      name, nullptr, size, PROT_READ | PROT_WRITE, low_4gb, false, &error_msg));
  CHECK(map_.get() != nullptr) << error_msg;
  memory_ = map_->Begin();
  static_assert(ArenaAllocator::kArenaAlignment <= kPageSize,
                "Arena should not need stronger alignment than kPageSize.");
  DCHECK_ALIGNED(memory_, ArenaAllocator::kArenaAlignment);
  size_ = map_->Size();
}
```
我们传入的参数我们标记下。
```

MemMap* MemMap::MapAnonymous(const char* name,       //传入参数
                             uint8_t* expected_ptr,  // nullptr
                             size_t byte_count,      // 传入参数
                             int prot,               // 传入内存区属性
                             bool low_4gb,           // 机器是否小于4G
                             bool reuse,             //是否要复用，此处no
                             std::string* error_msg, // 接收错误消息的指针
                             bool use_ashmem) {      //是否用ashmem，没有设，默认是true，我们用ashmem
#ifndef __LP64__
  UNUSED(low_4gb);
#endif
  use_ashmem = use_ashmem && !kIsTargetLinux;
  if (byte_count == 0) {
    return new MemMap(name, nullptr, 0, nullptr, 0, prot, false);
  }
  size_t page_aligned_byte_count = RoundUp(byte_count, kPageSize);

  int flags = MAP_PRIVATE | MAP_ANONYMOUS;
  if (reuse) {
    // reuse means it is okay that it overlaps an existing page mapping.
    // Only use this if you actually made the page reservation yourself.
    CHECK(expected_ptr != nullptr);

    DCHECK(ContainedWithinExistingMap(expected_ptr, byte_count, error_msg)) << *error_msg;
    flags |= MAP_FIXED;
  }

  if (use_ashmem) {
    if (!kIsTargetBuild) {
      // When not on Android (either host or assuming a linux target) ashmem is faked using
      // files in /tmp. Ensure that such files won't fail due to ulimit restrictions. If they
      // will then use a regular mmap.
      struct rlimit rlimit_fsize;
      CHECK_EQ(getrlimit(RLIMIT_FSIZE, &rlimit_fsize), 0);
      use_ashmem = (rlimit_fsize.rlim_cur == RLIM_INFINITY) ||
        (page_aligned_byte_count < rlimit_fsize.rlim_cur);
    }
  }

  unique_fd fd;


  if (use_ashmem) {
    // android_os_Debug.cpp read_mapinfo assumes all ashmem regions associated with the VM are
    // prefixed "dalvik-".
    std::string debug_friendly_name("dalvik-");
    debug_friendly_name += name;
    fd.reset(ashmem_create_region(debug_friendly_name.c_str(), page_aligned_byte_count));  // 通过ashmem驱动获取一块内存

    if (fd.get() == -1) {
      // We failed to create the ashmem region. Print a warning, but continue
      // anyway by creating a true anonymous mmap with an fd of -1. It is
      // better to use an unlabelled anonymous map than to fail to create a
      // map at all.
      PLOG(WARNING) << "ashmem_create_region failed for '" << name << "'";
    } else {
      // We succeeded in creating the ashmem region. Use the created ashmem
      // region as backing for the mmap.
      flags &= ~MAP_ANONYMOUS;
    }
  }

  // We need to store and potentially set an error number for pretty printing of errors
  int saved_errno = 0;

  void* actual = MapInternal(expected_ptr,
                             page_aligned_byte_count,
                             prot,
                             flags,
                             fd.get(),
                             0,
                             low_4gb); //将内存映射到进程空间
  saved_errno = errno;

  if (actual == MAP_FAILED) {
    if (error_msg != nullptr) {
      if (kIsDebugBuild || VLOG_IS_ON(oat)) {
        PrintFileToLog("/proc/self/maps", LogSeverity::WARNING);
      }

      *error_msg = StringPrintf("Failed anonymous mmap(%p, %zd, 0x%x, 0x%x, %d, 0): %s. "
                                    "See process maps in the log.",
                                expected_ptr,
                                page_aligned_byte_count,
                                prot,
                                flags,
                                fd.get(),
                                strerror(saved_errno));
    }
    return nullptr;
  }
  if (!CheckMapRequest(expected_ptr, actual, page_aligned_byte_count, error_msg)) {
    return nullptr;
  }
  return new MemMap(name, reinterpret_cast<uint8_t*>(actual), byte_count, actual,
                    page_aligned_byte_count, prot, reuse);  //构建成功
}
```

从上面代码我们可以看到，构建Linear AllocSpace的过程大概如下。通过ashmem驱动申请一块内存区域，将该区域映射到当前进程的进程空间，使用该内存。  


## 4.其他基础内存分配器
刚我们看到了Arena Alloc内存分配器，在art的源码目录art/runtime/base下还有别的内存分配器。  
arena_allocator、scoped_arena_allocator  

// Fast single-threaded allocator for zero-initialized memory chunks.
//
// Memory is allocated from ArenaPool in large chunks and then rationed through
// the ArenaAllocator. It's returned to the ArenaPool only when the ArenaAllocator
// is destroyed.
class ArenaAllocator

// Fast single-threaded allocator. Allocated chunks are _not_ guaranteed to be zero-initialized.
//
// Unlike the ArenaAllocator, ScopedArenaAllocator is intended for relatively short-lived
// objects and allows nesting multiple allocators. Only the top allocator can be used but
// once it's destroyed, its memory can be reused by the next ScopedArenaAllocator on the
// stack. This is facilitated by returning the memory to the ArenaStack.
class ScopedArenaAllocator

从两个的介绍我们可以知道他们的区别如下  
- ArenaAllocator
    分配的内存会被初始化。
    该分配器分配的内存是从ArenaPool中分配，中间有复用的过程，当ArenaAllocator分配器销毁后内存会被还给内存池ArenaPool。  
- ScopedArenaAllocator
    分配的内存不能保证被初始化过。
    相对于ArenaAllocator，ScopedArenaAllocator分配的内存一般存储生存时间较短的对象，这一点我们可以猜出他应该是用来创建Alloc Space内存区内存的。  
    分配器可以嵌套多个分配器，但是只有顶端分配器能用来分配，一旦顶端分配器被销毁，他分配的内存能被下一个栈上的ScopedArenaAllocator内存分配器使用，这一点让我们想起该内存分配器也可以做栈内存分配。
    栈应该是用ArenaStack来追踪的。

