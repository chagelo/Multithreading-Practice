#include <atomic>
#include <cassert>
#include <thread>

void TestReleaseAcquire() {
  int data = 0;
  std::atomic<int> flag = 0;
  std::thread t1([&]() {
    data = 42;
    flag.store(1, std::memory_order_release);
  });

  std::thread t2([&]() {
    while (!flag.load(std::memory_order_acquire))
      ;

    assert(data == 42);
  });

  t1.join();
  t2.join();
}
