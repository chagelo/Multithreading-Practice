#include <condition_variable>
#include <future>
#include <iostream>
#include <mutex>
#include <thread>

void task(int a, int b, std::promise<int> &ret) {
  int ret_a = a * a;
  int ret_b = b * 2;
  ret.set_value(ret_a + ret_b);
}

int main() {
  std::promise<int> p;
  std::future<int> f = p.get_future();

  std::thread t(task, 1, 2, std::ref(p));

  // do someting
  
  // get return value

  std::cout << "return value is " << f.get();

  // only once f.get(), multiple f.get() will crash

  t.join();
}