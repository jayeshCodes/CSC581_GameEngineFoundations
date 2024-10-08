#ifndef PACKED_MAP_HPP
#define PACKED_MAP_HPP

#include <iostream>
#include <map>
#include <queue>
#include <mutex>

template<typename K, typename V>
class PackedMap {
private:
    std::map<K, V> map;
    std::queue<K> availableSlots;
    V emptyValue;
    mutable std::mutex mutex;

public:
    PackedMap(const int capacity, V emptyValue)
        : emptyValue(emptyValue) {
        for (int i = 0; i < capacity; ++i) {
            K key = i;
            map[key] = emptyValue;
            availableSlots.push(key);
        }
    }

    int insert(V value) {
        std::lock_guard<std::mutex> lock(mutex);
        if (availableSlots.empty()) {
            std::cout << "No available slots for value " << value << std::endl;
            return -1;
        }
        K slot = availableSlots.front();
        availableSlots.pop();
        map[slot] = value;
        std::cout << "Value inserted at slot " << slot << std::endl;
        return slot;
    }

    void remove(K key) {
        std::lock_guard<std::mutex> lock(mutex);
        if (map.find(key) != map.end() && map[key] != emptyValue) {
            V value = map[key];
            map[key] = emptyValue;
            availableSlots.push(key);
            std::cout << "Value removed from slot " << key << std::endl;
        } else {
            std::cout << "Key " << key << " not found or already empty" << std::endl;
        }
    }

    int size() const {
        std::lock_guard<std::mutex> lock(mutex);
        return map.size();
    }

    V& operator[](const K& key) {
        std::lock_guard<std::mutex> lock(mutex);
        return map[key];
    }

    ~PackedMap() {
        std::lock_guard<std::mutex> lock(mutex);
        for (auto &entry: map) {
            if (entry.second != emptyValue) {
                remove(entry.first);
            }
        }
    }
};

#endif // PACKED_MAP_HPP