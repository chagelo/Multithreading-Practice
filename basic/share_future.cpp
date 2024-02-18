#include <condition_variable>
#include <future>
#include <iostream>
#include <mutex>
#include <thread>

void task(int a, std::future<int> &b, std::promise<int> &ret) {
  int ret_a = a * a;
  // block here util the future is set_value
  std::cout << "sleep position" << std::endl;
  int ret_b = b.get() * 2;
  ret.set_value(ret_a + ret_b);
}

int main() {
  std::promise<int> p;
  std::future<int> f = p.get_future();

  std::promise<int> p_in;
  std::future<int> f_in = p_in.get_future();

  // share the f_in in multiple child thread
  std::shared_future<int>s_f = f_in.share();

  // wait(sleep child thread at the position of calling the value) 
  std::thread t(task, 1, std::ref(f_in), std::ref(p));
  std::thread t1(task, 1, std::ref(f_in), std::ref(p));
  std::thread t2(task, 1, std::ref(f_in), std::ref(p));
  std::thread t3(task, 1, std::ref(f_in), std::ref(p));

  // do someting
  // .....
  p_in.set_value(2);
  // get return value

  std::cout << "return value is " << f.get();

  // only once f.get(), multiple f.get() will crash

  t.join();
}

/*
  promise and future is used to pass the data in different thread, when passing pointer is sometimes dangerous
  usage: thread 1 -> thread 2, thread initialize promise and future, and pass promise to thread, 
  it is the promise from thread 2 to thread 1, future is the result or accept the promise, than use the future to
  get the value from the thread 2
*/