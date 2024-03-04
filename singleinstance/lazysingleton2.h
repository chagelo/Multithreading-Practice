class Singleton {
 private:
  static Singleton* instance;

 private:
  Singleton(){};
  ~Singleton(){};
  Singleton(const Singleton&);
  Singleton& operator=(const Singleton&);

 public:
  class Deallocator {
   public:
    ~Deallocator() {
      if (instance) {
        delete instance;
        instance = nullptr;
      }
    }
  };
  static Deallocator deallocator;

 public:
  static Singleton* getInstance() {
    if (instance == nullptr) instance = new Singleton();
    return instance;
  }
};

// init static member
Singleton* Singleton::instance = nullptr;
Singleton::Deallocator Singleton::deallocator{};