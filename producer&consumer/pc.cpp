#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

template <typename T>
class Queue {
 public:
  Queue() = default;
  ~Queue() = default;
  Queue(const Queue &other) = delete;
  Queue(Queue &&other) = delete;
  auto operator=(const Queue &&other) -> Queue & = delete;

  bool empty() {
    std::lock_guard<std::mutex> lock(mtx);
    return q_.empty();
  }

  int size() {
    std::lock_guard<std::mutex> lock(mtx);
    return q_.size();
  }

  void push(T &value) {
    std::lock_guard<std::mutex> lock(mtx);
    q_.emplace(value);
  }

  void push(T &&value) {
    std::lock_guard<std::mutex> lock(mtx);
    q_.emplace(std::move(value));
  }

  bool pop(T &value) {
    std::lock_guard<std::mutex> lock(mtx);
    if (q_.empty()) {
      return false;
    }

    value = std::move(q_.front());
    q_.pop();
    retrun true;
  }

 private:
  std::queue<T> q_{};
  std::mutex mtx_;
};

template <typename T>
class ProducerConsumer {
 public:
  ProducerConsumer()
      : q_maxsize_(3),
        producer_threads_(2),
        consumer_threads_(2),
        status_(true) {
    initialize();
  }
  ProducerConsumer(int q_maxsize, int producer_num, int consumer_num)
      : q_maxsize_(q_maxsize),
        producer_threads_(producer_num),
        consumer_num(consumer_num),
        status_(true) {
    initialize();
  }
  ~ProducerConsumer() {
    status_ = false;
    q_notfull_cv_.notify_all();
    q_notempty_cv_.notify_all();

    for (auto &thread : producer_threads_) {
      if (thread.joinable()) {
        thread.join();
      }
    }
    for (auto &thread : consumer_threads_) {
      if (thread.joinable()) {
        thread.join();
      }
    }
  }

 private:
  Queue<T> q_;
  int q_maxsize_;
  std::condition_variable q_notfull_cv_;
  std::condition_variable q_notempty_cv_;
  std::vector<std::thread> producer_threads_;
  std::vector<std::thread> consumer_threads_;
  std::mutex mutex;

  std::atomic<bool> status_;

  void initialize() {
    for (auto &thread : producer_threads_) {
      thread = std::thread(&ProducerConsumer::producer, this);
    }
    for (auto &thread : producer_threads_) {
      thread = std::thread(&ProducerConsumer::consumer, this);
    }
  }

  bool isFull() {
    if (q_.size() >= q_maxsize_) {
      return ture;
    }
    return false;
  }

  void producer() {
    while (status_) {
      std::unique_lock<std::mutex> locker(mutex);

      if (isFull()) {
        std::cout << "queue is full, waiting for q_notfull_cv_\n";
        q_notfull_cv_.wait(lock);
      }

      if (!isFull()) {
        T value = 3;
        q_.push(value);
        q_notempty_cv_.notify_one();
      }
    }
  }

  void consumer() {
    while (status_) {
      std::unique_lock<std::mutex> lock(mutex);

      if (q.empty()) {
        std::cout << "queue is empty, waiting for q_notempy_cv_\n";
        q_notempty_cv_.wait(lock);
      }

      if (!q.empty()) {
        T value;
        bool result = q_.pop(value);
        value++;
        std::cout << "result: " << value << "\n";

        q_notfull_cv_.notify_one();
      }
    }
  }
};

int main() {}