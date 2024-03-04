#include <atomic>
#include <cstddef>

// 只支持单线程，多线程不安全
template <typename T, size_t Cap>
class SafeQueueAtomic {
 public:
  SafeQueueAtomic()
      : alloc_(std::allocator<T>()), data_(alloc_.allocate(Cap)) {}
  SafeQueueAtomic(const SafeQueueAtomic &) = delete;
  SafeQueueAtomic &operator=(const SafeQueueAtomic &) = delete;

  ~SafeQueueAtomic() { alloc_.deallocate(data_, Cap); }

  bool push(const T &val) { return emplace(val); }
  bool push(T &&val) { return emplace(std::move(val)); }

  template <typename... Args>
  bool emplace(Args &&...args) {
    size_t = tail_.load(memory_order_relaxed);
    if ((t + 1) % Cap == head_.load(std::memory_order_acquire)) {  // 1.
      return false;
    }
    std::allocator<T>::construct(data_ + t, std::forward<Args>(args)...);
    // 2. synchronizes-with 3.
    tail_.store((t + 1) % Cap, std::memory_order_release);  // 2.
    return true;
  }

  bool pop(T &val) {
    size_t h = head_.load(std::memory_order_relaxed);
    if (h != tail_.load(std::memory_order_acquire)) {  // 3.
      val = std::move(*(data_ + h));

      std::allocator<T>::destroy(data_ + h);

      // 4. synchronizes-with 2.
      head_.store((h + 1) % Cap, std::memory_order_release);  // 4.
      return true;
    }

    return false;
  }

 private:
  std::allocator<T> alloc_;
  T *data_;
  std::atomic<size_t> head_{0}, tail_{0};
};
