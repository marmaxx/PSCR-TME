#pragma once

#include <vector>
#include <forward_list>
#include <utility>
#include <cstddef>

template<typename K, typename V>
class HashMap {
public:
    // Entry stores a const key and a mutable value
    struct Entry {
        const K key_;
		V value_;
		Entry(const K& key, const V& value) : key_(key), value_(value) {}
    };

    using Bucket = std::forward_list<Entry>;
    using Table  = std::vector<Bucket>;

    // Construct with a number of buckets (must be >= 1)
    HashMap(std::size_t nbuckets = 1024) : buckets_(nbuckets) {};

    // Return pointer to value associated with key, or nullptr if not found.
    // Only iterate the appropriate bucket.
    V* get(const K& key) {
        size_t h = std::hash<K>()(key);
        size_t bucket_index = h % buckets_.size();
        for (auto& entry : buckets_[bucket_index]) {
            if (entry.key_ == key) return &entry.value_;   
        }
        return nullptr;
    }

    // Insert or update (key,value).
    // Returns true if an existing entry was updated, false if a new entry was inserted.
    bool put(const K& key, const V& value) {
        size_t h = std::hash<K>()(key);
        size_t bucket_index = h % buckets_.size();
        for (auto& entry : buckets_[bucket_index]) {
            if (entry.key_ == key) {
                entry.value_ = value;
                return true;
            }
        }
        buckets_[bucket_index].emplace_front(key, value);
        count_++;
        return false;
    }

    // Current number of stored entries
    std::size_t size() const { return count_; }

    // Convert table contents to a vector of key/value pairs.
    std::vector<std::pair<K,V>> toKeyValuePairs() const {
        std::vector<std::pair<K,V>> result;
        for (const auto& bucket : buckets_) {
            for (const auto& entry : bucket) {
                result.emplace_back(entry.key_, entry.value_);
            }
        }
        return result;
    };

    // Optional: number of buckets
    // std::size_t bucket_count() const;

private:
    Table buckets_;
    std::size_t count_ = 0;
};
