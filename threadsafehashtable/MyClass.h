#include <iostream>

class MyClass {
 public:
  MyClass(int i) : data_(i) {}
  friend std::ostream& operator<<(std::ostream& os, const MyClass& mc) {
    os << mc.data_;
    return os;
  }

 private:
  int data_;
};