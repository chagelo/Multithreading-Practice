#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
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
    std::lock_guard<std::mutex> lock(mtx_);
    return q_.empty();
  }

  int size() {
    std::lock_guard<std::mutex> lock(mtx_);
    return q_.size();
  }

  void push(T &value) {
    std::lock_guard<std::mutex> lock(mtx_);
    q_.emplace(value);
  }

  void push(T &&value) {
    std::lock_guard<std::mutex> lock(mtx_);
    q_.emplace(std::move(value));
  }

  bool pop(T &value) {
    std::lock_guard<std::mutex> lock(mtx_);
    if (q_.empty()) {
      return false;
    }

    value = std::move(q_.front());
    q_.pop();
    return true;
  }

 private:
  std::queue<T> q_{};
  std::mutex mtx_;
};

class ThreadPool {
 public:
  ThreadPool(const ThreadPool &) = delete;
  auto operator=(const ThreadPool &) -> ThreadPool & = delete;
  ThreadPool(ThreadPool &&) = delete;
  auto operator=(ThreadPool &&) -> ThreadPool & = delete;

  ThreadPool() : threads_(std::thread::hardware_concurrency()), status_(true) {
    initialize();
  }
  ThreadPool(int thread_num) : threads_(thread_num), status_(true) {
    initialize();
  }
  ~ThreadPool() {
    status_ = false;
    cv_.notify_all();
    for (auto &thread : threads_) {
      if (thread.joinable()) {
        thread.join();
      }
    }
  }

  template <typename Func, typename... Args>
  auto submitTask(Func &&func, Args... args) {
    using return_type = typename std::invoke_result<Func, Args...>::type;
    std::function<return_type()> task_wrapper1 =
        std::bind(std::forward<Func>(func), std::forward<Args>(args)...);

    auto task_wrapper2 =
        std::make_shared<std::packaged_task<return_type()>>(task_wrapper1);
    TaskType wrapper_function = [task_wrapper2]() { (*task_wrapper2)(); };

    q_.push(wrapper_function);
    cv_.notify_one();

    return task_wrapper2->get_future();
  }

 private:
  using TaskType = std::function<void()>;

  Queue<TaskType> q_;
  std::vector<std::thread> threads_;
  std::condition_variable cv_;
  std::mutex mutex_;
  std::atomic<bool> status_;

  void initialize() {
    int index = 0;
    for (auto &thread : threads_) {
      auto worker = [this, index]() {
        while (status_) {
          TaskType task;
          bool isSuccess = false;
          {
            std::unique_lock<std::mutex> lock(this->mutex_);
            while (this->q_.empty()) {
              this->cv_.wait(lock);
            }
            isSuccess = this->q_.pop(task);
          }

          if (isSuccess) {
            std::cout << "Start running task in worker: [ID]" << index << "\n";
            task();
            std::cout << "End running task in worker: [ID]" << index << "\n";
          }
        }
      };
      ++index;
      thread = std::thread(worker);
    }
  }
};

// test
int very_time_consuming_task(int a, int b) {
  std::this_thread::sleep_for(std::chrono::seconds(1));
  return a + b;
}

int main() {
  ThreadPool thread_pool(12);

  int task_num = 30;

  std::vector<std::future<int>> result(30);

  std::cout << "Start to submit tasks...\n";
  for (size_t i = 0; i < task_num; ++i) {
    result[i] = thread_pool.submitTask(very_time_consuming_task, i, i + 1);
  }
  std::cout << "End submit tasks...\n";

  std::cout << "Main thread do something else ...\n";

  std::this_thread::sleep_for(std::chrono::seconds(10));

  std::cout << "Main thread task finished...\n";

  for (size_t i = 0; i < task_num; ++i) {
    std::cout << "result[" << i << "]" << result[i].get();
  }

  std::cout << "End of getting result\n";
}