// version 1.2
class Singleton {
 private:
  Singleton(){};
  ~Singleton(){};
  Singleton(const Singleton&);
  Singleton& operator=(const Singleton&);

 public:
  static Singleton& getInstance() {
    static Singleton instance;
    return instance;
  }
};