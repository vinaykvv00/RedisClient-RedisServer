#include "../include/RedisDatabase.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

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
File -> key : load()

*/

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
