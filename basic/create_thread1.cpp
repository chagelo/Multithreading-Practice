#include <chrono>
#include <iostream>
#include <thread>

void func(int a) {
    while (true) {
        std::cout << "Hello World!\n";
        std::this_thread::sleep_for(std::chrono::microseconds(50));
    }
}

int main() {
    int a = 0;
    // create thread and run
    std::thread thread1(func, a);

    // main thread block here, wait child thread over
    thread1.join();

    thread1.get_id();

    std::thread thread2(func, 2);
    thread1.swap(thread2);
}