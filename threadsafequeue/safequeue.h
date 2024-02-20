#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>

template <typename T>
class SafeQueue {
 public:
  SafeQueue() {}

  void push(T new_value) {
    std::lock_guard<std::mutex> lock(mtx_);
    q_.emplace(std::move(new_value));
    cv_.notify_one();
  }

  void wait_and_pop(T& value) {
    std::unique_lock<std::mutex> lock(mtx_);
    cv_.wait(lock, [this]() { return !q_.empty(); });
    value = std::move(q_.front());
    q_.pop();
  }

  std::shared_ptr<T> wait_and_pop() {
    std::unique_lock<std::mutex> lock(mtx_);
    cv_.wait(lock, [this]() { return !q_.empty(); });
    std::shared_ptr<T> res(std::make_shared<T>(std::move(q_.front())));
    q_.pop();
    return res;
  }

  bool try_pop(T& value) {
    std::lock_guard<std::mutex> lock(mtx_);
    if (q_.empty()) {
      return false;
    }
    value = std::move(q_.front());
    q_.pop();
    return true;
  }

  std::shared_ptr<T> try_pop() {
    std::lock_guard<std::mutex> lock(mtx_);
    if (q_.empty()) {
      return std::shared_ptr<T>();
    }
    std::shared_ptr<T> res(std::make_shared<T>(std::move(q_.front())));
    q_.pop();
    return res;
  }

  // 加了const的成员函数可以被非const对象和const对象调用, const this = &object,
  // const this = &const object 但不加const的成员函数只能被非const对象调用, this
  // = &const object, error
  bool empty() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return q_.empty();
  }

 private:
  std::mutex mtx_;
  std::queue<T> q_;
  std::condition_variable cv_;
};

template <typename T>
class SafeQueuePtr {
 public:
  SafeQueuePtr() {}

  // 阻塞方式的 pop
  void wait_and_pop(T& value) {
    std::unique_lock<std::mutex> lock(mtx_);
    cv_.wait(lock, [this] { return !q_.empty(); });
    value = std::move(*q_.front());
    q_.pop();
  }

  // 非阻塞方式的 pop
  bool try_pop(T& value) {
    std::lock_guard<std::mutex> lock(mtx_);
    if (!q_.empty()) {
      return false;
    }

    value = std::move(*q_.front());
    q_.pop();
  }

  // 阻塞方式的 pop
  std::shared_ptr<T> wait_and_pop() {
    std::unique_lock<std::mutex> lock(mtx_);
    cv_.wait(lock, [this] { return !q_.empty(); });
    std::shared_ptr<T> res(q_.front());
    q_.pop();
    return res;
  }

  // 非阻塞方式的 pop
  std::shared_ptr<T> try_pop() {
    std::lock_guard<std::mutex> lock(mtx_);
    if (q_.empty()) {
      return nullptr;
    }

    std::shared_ptr<T> res(q_.front());
    q_.pop();
    return res;
  }

  void push(T new_value) {
    std::shared_ptr<T> temp(std::make_shared<T>(std::move(new_value)));
    std::lock_guard<std::mutex> lock(mtx_);
    q_.emplace(temp);
    cv_.notify_one();
  }
  bool empty() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return q_.empty();
  }

 private:
  std::mutex mtx_;
  std::queue<std::shared_ptr<T>> q_;
  std::condition_variable cv_;
};

// use custom list to implement queue instead of stl queue
template <typename T>
class SafeQueueList {
 public:
  SafeQueueList() : head_(new ListNode), tail_(head_.get()) {}

  SafeQueueList(const SafeQueueList&) = delete;
  auto operator=(const SafeQueueList&) -> SafeQueueList& = delete;

  std::shared_ptr<T> wait_and_pop() {
    std::unique_ptr<ListNode> const old_head = wait_pop_head();
    return old_head->data_;
  }

  void wait_and_pop(T& value) {
    std::unique_ptr<ListNode> const old_head = wait_pop_head(value);
  }

  std::shared_ptr<T> try_pop() {
    std::unique_ptr<ListNode> old_head = try_pop_head();
    return old_head ? old_head->data_ : nullptr;
  }

  bool try_pop(T& value) {
    std::unique_ptr<ListNode> const old_head = try_pop_head(value);
    if (old_head) {
      return true;
    }
    return false;
  }

  bool empty() {
    std::scoped_lock<std::mutex, std::mutex>(head_mtx_, tail_mtx_);
    return head_.get() == tail_;
  }

  void push(T new_value) {
    std::shared_ptr<T> new_data(std::make_shared<T>(std::move(new_value)));
    std::unique_ptr<ListNode> new_node(std::make_unique<ListNode>());
    {
      std::lock_guard<std::mutex> tail_lock(tail_mtx_);

      ListNode* const new_tail = new_node.get();
      tail_->data_ = new_data;
      tail_->next_ = std::move(new_node);
      tail_ = new_tail;
    }
    cv_.notify_one();
  }

 private:
  struct ListNode {
    std::shared_ptr<T> data_;
    std::unique_ptr<ListNode> next_;
  };

  std::mutex head_mtx_;
  std::mutex tail_mtx_;
  std::unique_ptr<ListNode> head_;
  ListNode* tail_;
  std::condition_variable cv_;

  ListNode* get_tail() {
    std::lock_guard<std::mutex> tail_lock(tail_mtx_);
    return tail_;
  }

  std::unique_ptr<ListNode> pop_head() {
    std::unique_ptr<ListNode> old_head = std::move(head_);
    head_ = std::move(old_head->next_);
    return old_head;
  }

  std::unique_lock<std::mutex> wait_for_data() {
    std::unique_lock<std::mutex> head_lock(head_mtx_);
    cv_.wait(head_lock, [this] { return head_.get() != get_tail(); });
    return std::move(head_lock);
  }

  std::unique_ptr<ListNode> wait_pop_head() {
    std::unique_lock<std::mutex> head_lock(wait_for_data());
    return pop_head();
  }

  std::unique_ptr<ListNode> wait_pop_head(T& value) {
    std::unique_lock<std::mutex> head_lock(wait_for_data());
    value = std::move(*head_->data_);
    return pop_head();
  }

  std::unique_ptr<ListNode> try_pop_head() {
    std::scoped_lock<std::mutex, std::mutex> head_tail_lock(head_mtx_,
                                                            tail_mtx_);
    if (head_.get() == tail_) {
      return nullptr;
    }
    return pop_head();
  }

  std::unique_ptr<ListNode> try_pop_head(T& value) {
    std::scoped_lock<std::mutex, std::mutex> head_tail_lock(head_mtx_,
                                                            tail_mtx_);
    if (head_.get() == tail_) {
      return nullptr;
    }
    value = std::move(*head_->data_);
    return pop_head();
  }
};
