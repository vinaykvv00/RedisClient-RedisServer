#include "../include/RedisDatabase.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

RedisDatabase& RedisDatabase::getInstance() {
    static RedisDatabase instance;
    return instance;
}

// key/value operations
// list and hash operations

/*
Memory -> file : dump()
File -> memory : load()
Key -> file : dump()
K key -> file : dump()
L list -> file : dump()
H hash -> file : dump()
File -> key : load()
*/

// common commands
bool RedisDatabase::flushAll() {
    std::lock_guard<std::mutex> lock(db_mutex);
    kv_store.clear();
    list_store.clear();
    hash_store.clear();
    expire_map.clear();
    return true;
}

void RedisDatabase::set(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(db_mutex);
    kv_store[key] = value;
}

bool RedisDatabase::get(const std::string& key, std::string& value) {
    std::lock_guard<std::mutex> lock(db_mutex);
    auto it = kv_store.find(key);
    if (it != kv_store.end()) {
        value = it->second;
        return true;
    }
    return false;
}

std::vector<std::string> RedisDatabase::keys() {
    std::lock_guard<std::mutex> lock(db_mutex);
    std::vector<std::string> result;
    for (const auto& key : kv_store) {
        result.push_back(key.first);
    }
    for (const auto& key : list_store) {
        result.push_back(key.first);
    }
    for (const auto& key : hash_store) {
        result.push_back(key.first);
    }
    return result;
}

std::string RedisDatabase::type(const std::string& key) {
    std::lock_guard<std::mutex> lock(db_mutex);
    if (kv_store.find(key) != kv_store.end()) {
        return "string";
    }
    if (list_store.find(key) != list_store.end()) {
        return "list";
    }
    if (hash_store.find(key) != hash_store.end()) {
        return "hash";
    }
    return "none";
}

bool RedisDatabase::del(const std::string& key) {
    std::lock_guard<std::mutex> lock(db_mutex);
    bool erased = false;
    erased |= kv_store.erase(key) > 0;
    erased |= list_store.erase(key) > 0;
    erased |= hash_store.erase(key) > 0;
    expire_map.erase(key);
    return erased;
}

bool RedisDatabase::expire(const std::string& key, int seconds) {
    std::lock_guard<std::mutex> lock(db_mutex);
    bool exists = kv_store.find(key) != kv_store.end() || list_store.find(key) != list_store.end() || hash_store.find(key) != hash_store.end();
    if (!exists) {
        return false;
    }
    expire_map[key] = std::chrono::steady_clock::now() + std::chrono::seconds(seconds);
    return true;
}

bool RedisDatabase::rename(const std::string& oldKey, const std::string& newKey) {
    std::lock_guard<std::mutex> lock(db_mutex);
    bool exists = kv_store.find(oldKey) != kv_store.end() || list_store.find(oldKey) != list_store.end() || hash_store.find(oldKey) != hash_store.end();
    if (!exists) {
        return false;
    }
    if (kv_store.find(oldKey) != kv_store.end()) {
        kv_store[newKey] = kv_store[oldKey];
        kv_store.erase(oldKey);
    }
    if (list_store.find(oldKey) != list_store.end()) {
        list_store[newKey] = list_store[oldKey];
        list_store.erase(oldKey);
    }
    if (hash_store.find(oldKey) != hash_store.end()) {
        hash_store[newKey] = hash_store[oldKey];
        hash_store.erase(oldKey);
    }
    if (expire_map.find(oldKey) != expire_map.end()) {
        expire_map[newKey] = expire_map[oldKey];
        expire_map.erase(oldKey);
    }
    return true;
}

bool RedisDatabase::dump(const std::string& filename) {
    std::lock_guard<std::mutex> lock(db_mutex);
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return false;
    }

    for (const auto& kv : kv_store) {
        file << kv.first << " " << kv.second << "\n";
    }
    for (const auto& kv : list_store) {
        file << kv.first;
        for (const auto& item : kv.second) {
            file << " " << item;
        }
        file << "\n";
    }
    for (const auto& kv : hash_store) {
        file << kv.first;
        for (const auto& item : kv.second) {
            file << " " << item.first << ":" << item.second;
        }
        file << "\n";
    }

    return true;
}
/*
examples of dump file format:
key1 value1
key2 value2
list1 item1 item2 item3
hash1 field1:value1 field2:value2

key-value (K)
kv_store[name] = "Alice"
list_store["list1"] = {"item1", "item2", "item3"};
hash_store["hash1"] = {{"field1", "value1"}, {"field2", "value2"}};
hash_store["user:100"] = {
    {"name", "Alice"},
    {"email", "alice@example.com"}
};
*/

bool RedisDatabase::load(const std::string& filename) {
    std::lock_guard<std::mutex> lock(db_mutex);
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return false;
    }

    kv_store.clear();
    list_store.clear();
    hash_store.clear();

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::istringstream iss(line);
        std::string type;
        iss >> type;
        if (type == "K") {
            std::string key, value;
            iss >> key >> value;
            kv_store[key] = value;
        } else if (type == "L") {
            std::string key;
            iss >> key;
            std::vector<std::string> items;
            std::string item;
            while (iss >> item) {
                items.push_back(item);
            }
            list_store[key] = items;
        } else if (type == "H") {
            std::string key;
            iss >> key;
            std::unordered_map<std::string, std::string> hash;
            std::string field_value;
            while (iss >> field_value) {
                auto pos = field_value.find(':');
                if (pos != std::string::npos) {
                    std::string field = field_value.substr(0, pos);
                    std::string value = field_value.substr(pos + 1);
                    hash[field] = value;
                }
            }
            hash_store[key] = hash;
        }
    }

    return true;
}
