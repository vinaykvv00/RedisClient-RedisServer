#ifndef REDIS_DATABASE_HPP
#define REDIS_DATABASE_HPP

#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

class RedisDatabase {
public:
    // Add your database methods here
    static RedisDatabase& getInstance();

    // common commands
    bool flushAll();

    // key value operations
    void set(const std::string& key, const std::string& value);
    bool get(const std::string& key, std::string& value);

    std::vector<std::string> keys();
    std::string type(const std::string& key);
    bool del(const std::string& key);

    // Expire operations
    bool expire(const std::string& key, int seconds);

    // rename
    bool rename(const std::string& oldKey, const std::string& newKey);

    // persistence Dump/load db from the file
    bool dump(const std::string& filename);
    bool load(const std::string& filename);

private:
    RedisDatabase() = default;
    ~RedisDatabase() = default;
    RedisDatabase(const RedisDatabase&) = delete;
    RedisDatabase& operator=(const RedisDatabase&) = delete;

    std::mutex db_mutex;
    std::unordered_map<std::string, std::string> kv_store;
    std::unordered_map<std::string, std::vector<std::string>> list_store;
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> hash_store;

    std::unordered_map<std::string, std::chrono::steady_clock::time_point> expire_map;
};

#endif  // REDIS_DATABASE_HPP
