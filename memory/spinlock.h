#include <atomic>
#include <iostream>
#include <thread>

class SpinLock {
 public:
  void lock() {
    // 自选等待，直到获取锁
    while (flag.test_and_set(std::memory_order_acquire))
      ;
  }

  void unlock() {
    // 释放锁
    flag.clear(std::memory_order_release);
  }

 private:
  // 设置一个初始值，无效状态
  std::atomic_flag flag = ATOMIC_FLAG_INIT;
};

void TestSpinLock() {
  SpinLock spinlock;

  std::thread t1([&spinlock]() {
    spinlock.lock();
    for (int i = 0; i < 3; ++i) {
      std::cout << "*";
    }
    std::cout << std::endl;
    spinlock.unlock();
  });

  std::thread t2([&spinlock]() {
    spinlock.lock();
    for (int i = 0; i < 3; ++i) {
      std::cout << "?";
    }
    std::cout << std::endl;
    spinlock.unlock();
  });

  t1.join();
  t2.join();
}
