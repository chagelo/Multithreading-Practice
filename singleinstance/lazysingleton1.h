#include <memory>


class Singleton {
 private:
  static std::shared_ptr<Singleton> instance;

 private:
  Singleton(){};
  ~Singleton(){};
  Singleton(const Singleton&);
  Singleton& operator=(const Singleton&);

 public:
  static std::shared_ptr<Singleton> getInstance() {
    if (instance == nullptr) instance = std::make_shared<Singleton>();
    return instance;
  }
};

// init static member
std::shared_ptr<Singleton> instance = nullptr;
