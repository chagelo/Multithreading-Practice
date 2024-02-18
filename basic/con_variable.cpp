#include <atomic>
#include <condition_variable>
#include <deque>
#include <iostream>
#include <mutex>
#include <thread>

std::mutex mtx;
std::deque<int> dq;

std::condition_variable cv;

// producer
void task1() {
  int i = 0;
  while (true) {
    std::lock_guard<std::mutex> lock(mtx);

    dq.push_back(i);

    cv.notify_one();
    if (i < 999999) {
      i++;
    } else {
      i = 0;
    }
    // std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}

// consumer
void task2() {
  int data = 0;
  while (true) {
    // std::lock_guard<std::mutex> lock(mtx);
    std::unique_lock<std::mutex> lock(mtx);

    while (dq.empty()) {
      // lock.unlock(), then sleep current thread
      cv.wait(lock);
    }

    data = dq.front();
    dq.pop_front();
    std::cout << "Get value from queue: " << data << std::endl;
    // std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}

int main() {
  std::thread t1(task1);
  std::thread t2(task2);

  t1.join();
  t2.join();
}