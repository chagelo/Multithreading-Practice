#include <atomic>
#include <iostream>
#include <mutex>
#include <thread>

std::mutex mtx;

int globalVariable = 0;
// std::atomic<int> globalVariable = 0;

// complex, may throw out error
void callAFunc() {}

void task1() {
  for (int i = 0; i < 1000000; ++i) {
    mtx.lock();

    // RAII
    // std::lock_guard<std::mutex> lock(mtx)

    // RAII, single mutext, but can unlock manually
    // std::unique_lock<std::mutex> lock(mtx);
    // lock.unlock();

    globalVariable++;
    globalVariable--;

    callAFunc();
    // 1. callAFunc(); but throw error, than not unlock
    // cause deadlock, so use RAII

    // 2. apply two lock, the first task apply lock1, lock2, the second task
    // apply lock2

    mtx.unlock();
  }
}

std::mutex mtx1, mtx2;

void task2() {
  for (int i = 0; i < 100000; ++i) {
    mtx1.lock();
    mtx2.lock();

    globalVariable++;
    globalVariable--;

    callAFunc();

    mtx1.unlock();
    mtx2.unlock();
  }
}

void task3() {
  for (int i = 0; i < 100000; ++i) {
    mtx2.lock();
    mtx1.lock();

    // lock the many mutex at the same time, need to call unlock manually
    // std::lock (mtx1, mtx2);
    // unlock: mtx1.unlock(); mtx2.unlock();

    // RAII, multiple mutex
    // std::scoped_lock<std::mutex, std::mutex>lock (mtx1, mtx2);

    globalVariable++;
    globalVariable--;

    callAFunc();

    mtx1.unlock();
    mtx2.unlock();
  }
}

int main() {
  std::thread t1(task1);
  std::thread t2(task1);

  t1.join();
  t2.join();

  std::cout << "current value is" << globalVariable;
}