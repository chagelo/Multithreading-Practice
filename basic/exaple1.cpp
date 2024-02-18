
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>

std::mutex mtx;
std::condition_variable cv;

void task(int a, int b, int &ret) {
  std::lock_guard<std::mutex> lock(mtx);
  int ret_a = a * a;
  int ret_b = b * 2;
  ret = ret_a + ret_b;
  cv.notify_one();
}

int main() {
  int ret = 0;
  std::thread t(task, 1, 2, std::ref(ret));

  std::unique_lock<std::mutex> lock(mtx);
  // we can not access the value when the task not finish, so here sleep
  cv.wait(lock);
  std::cout << "return value is" << ret;

  t.join();
}

/*

void f(int& n1, int& n2, const int& n3) {
  std::cout << "In function: " << n1 << ' ' << n2 << ' ' << n3 << '\n';
  ++n1;  // increments the copy of n1 stored in the function object
  ++n2;  // increments the main()'s n2
         // ++n3; // compile error
}

int main() {
  int n1 = 1, n2 = 2, n3 = 3;
  std::function<void()> bound_f = std::bind(f, n1, std::ref(n2), std::cref(n3));
  n1 = 10;
  n2 = 11;
  n3 = 12;
  std::cout << "Before function: " << n1 << ' ' << n2 << ' ' << n3 << '\n';
  bound_f();
  std::cout << "After function: " << n1 << ' ' << n2 << ' ' << n3 << '\n';
}

1. std::ref usage, in functional programming, parameter is copy not ref

such as

template <typename Fn, typename... Args>
auto call_by_value(Fn &&fn, Args... args) {
  return fn(args...);
}
void func(int &a) { a = 1; }
int main() {
  int a = 0;
  call_by_value(func, a); // here, a is copy not ref
  std::cout << a << std::endl;
}

2. std::ref use in std::thread, multiple thread share variable

*/