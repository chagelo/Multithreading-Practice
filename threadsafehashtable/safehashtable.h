#include <algorithm>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <vector>

template <typename Key, typename Value, typename Hash = std::hash<Key>>
class SafeHashTable {
 public:
  SafeHashTable(unsigned num_buckets = 19, const Hash& hash_func = Hash())
      : buckets_(num_buckets), hash_func_(hash_func) {
    for (unsigned i = 0; i < num_buckets; ++i) {
      buckets_[i].reset(new bucket());
    }
  }
  SafeHashTable(const SafeHashTable& other) = delete;
  auto operator=(SafeHashTable&& other) -> SafeHashTable& = delete;

  Value value_for(const Key& key, const Value& default_value = Value()) {
    return get_bucket(key).value_for(key, default_value);
  }

  void update(const Key& key, const Value& value) {
    return get_bucket(key).update_bucket(key, value);
  }

  void remove(const Key& key) { return get_bucket(key).remove(key); }

  // get all (key, value)
  std::map<Key, Value> get_map() {
    std::vector<std::unique_lock<std::shared_mutex>> locks;
    for (unsigned i = 0; i < buckets_.size(); ++i) {
      locks.emplace_back(
          std::unique_lock<std::shared_mutex>(buckets_[i]->mutex_));
    }

    std::map<Key, Value> res;
    for (unsigned i = 0; i < buckets_.size(); ++i) {
      for (auto it = buckets_[i]->data_.begin(); it != buckets_[i]->data_.end();
           ++it) {
        res.insert(*it);
      }
    }
    return res;
  }

 private:
  class bucket {
    friend class SafeHashTable;

   public:
    // search the (k, v) in bucket
    Value value_for(const Key& key, const Value& default_value = Value()) {
      std::shared_lock<std::shared_mutex> lock(mutex_);
      auto entry = find_entry_for(key);
      return (entry == data_.end() ? default_value : entry->second);
    }

    // update bucket
    void update_bucket(const Key& key, const Value& value) {
      std::unique_lock<std::shared_mutex> lock(mutex_);
      auto entry = find_entry_for(key);
      if (entry == data_.end()) {
        data_.emplace_back(bucket_value(key, value));
      } else {
        entry->second = value;
      }
    }

    void remove(const Key& key) {
      std::unique_lock<std::shared_mutex> lock(mutex_);
      auto entry = find_entry_for(key);
      if (entry != data_.end()) {
        data_.erase(entry);
      }
    }

   private:
    typedef std::pair<Key, Value> bucket_value;
    typedef std::list<bucket_value> bucket_data;
    typedef typename bucket_data::iterator bucket_iterator;

    bucket_data data_;
    std::shared_mutex mutex_;
    bucket_iterator find_entry_for(const Key& key) {
      return std::find_if(data_.begin(), data_.end(), [&](bucket_value const& item) {
        return item.first == key;
      });
    }
  };

  std::vector<std::unique_ptr<bucket>> buckets_;

  Hash hash_func_;

  bucket& get_bucket(const Key& key) const {
    const std::size_t bucket_index = hash_func_(key) % buckets_.size();
    return *(buckets_[bucket_index]);
  }
};
