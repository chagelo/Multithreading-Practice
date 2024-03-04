// version 1.3
class Singleton {
 private:
  static Singleton instance;

 private:
  Singleton();
  ~Singleton();
  Singleton(const Singleton&);
  Singleton& operator=(const Singleton&);

 public:
  static Singleton& getInstance() { return instance; }
};

// initialize defaultly
Singleton Singleton::instance;