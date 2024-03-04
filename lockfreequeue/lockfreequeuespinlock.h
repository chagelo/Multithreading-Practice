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
    // SpinLock.lock()
    size_t t = tail_;
    if ((t + 1) % Cap == head_) {  // 1.
      return false;
    }
    std::allocator<T>::construct(data_ + t, std::forward<Args>(args)...);
    tail_ = (tail_ + 1) % Cap;  // 2.
    // SpinLock.unlock()
    return true;
  }

  bool pop(T &val) {
    // SpinLock.lock()
    if (head_ != tail_) {  // 3.
      val = std::move(*(data_ + h));

      std::allocator<T>::destroy(data_ + h);

      head_ = (head_ + 1) % Cap;  // 4.
      // SpinLock.unlock()
      return true;
    }
    // SpinLock.unlock()
    return false;
  }

 private:
  std::allocator<T> alloc_;
  T *data_;
  int head_{0}, tail_{0};
};
