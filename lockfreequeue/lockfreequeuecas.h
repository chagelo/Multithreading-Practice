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

  bool push(T &&val) {
    size_t, w;
    do {
      t = tail_.load();
      if ((t + 1) % Cap = head_.load()) {
        return false;
      }
    } while (!tail_.compare_exchange_strong(t, (t + 1 % Cap)));
    data[t] = val;
    do {
      w = t;
    } while (!write_.compare_exchange_strong(w, (w + 1) % Cap));
    return true;
  }

  bool pop(T &val) {
    do {
      int h = head_.load();
      if (h == write_.load()) {
        return false;
      }
      val = data_[h];
    } while (head_.compare_exchange_strong(h, (h + 1) % Cap));
  }

 private:
  std::allocator<T> alloc_;
  T *data_;
  std::atomic<int> head_{0}, tail_{0}, write_{0};
};
