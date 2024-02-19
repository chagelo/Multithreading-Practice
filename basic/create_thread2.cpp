#include <chrono>
#include <iostream>
#include <thread>

void func(int a, int &b, int &&c) {
  if (true) {
    std::cout << "Hello World!\n";
    std::this_thread::sleep_for(std::chrono::microseconds(50));
  }
  a = 10, b = 11, c = 12;
}

class Worker {
 public:
  void funcA() {}
};

class TestA {
 public:
  void operator()(){};
};

int main() {
  int a = 0;
  int b = 1;
  int c = 2;
  std::cout << a << " " << b << " " << c << std::endl;
  // create thread and run
  std::thread thread1(func, a, std::ref(b), c);

  // main thread block here, wait child thread over
  thread1.join();
  std::cout << a << " " << b << " " << c << std::endl;

  // member function
  Worker w;
  std::thread thread2(&Worker::funcA, &w);

  // lambda function
  // ...

  // error: std::thread thread3(TestA());
  std::thread thread3((TestA()));
  std::thread thread4 { TestA() };

}