#include <condition_variable>
#include <future>
#include <iostream>
#include <mutex>
#include <thread>

int task(int a, int b) {
  int ret_a = a * a;
  int ret_b = b * 2;
  return ret_a + ret_b;
}

int main() {
  std::future<int> fu = std::async(task, 1, 2);
}
