#ifndef REDIS_DATABASE_HPP
#define REDIS_DATABASE_HPP

#include <chrono>
#include <mutex>
#include <sys/types.h>
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
    bool expire(const std::string& key, int seconds);
    bool rename(const std::string& oldKey, const std::string& newKey);

    // persistence Dump/load db from the file
    bool dump(const std::string& filename);
    bool load(const std::string& filename);

    // List operations
    enum class ListResult { Ok, Missing, OutOfRange, WrongType };

    // A negative integer result indicates a wrong-type key.
    ssize_t llen(const std::string& key);
    ssize_t lpush(const std::string& key, const std::string& value);
    ssize_t rpush(const std::string& key, const std::string& value);
    ListResult lpop(const std::string& key, std::string& value);
    ListResult rpop(const std::string& key, std::string& value);
    ssize_t lrem(const std::string& key, int count, const std::string& value);
    ListResult lindex(const std::string& key, int index, std::string& value);
    ListResult lset(const std::string& key, int index, const std::string& value);

private:
    RedisDatabase() = default;
    ~RedisDatabase() = default;
    RedisDatabase(const RedisDatabase&) = delete;
    RedisDatabase& operator=(const RedisDatabase&) = delete;

    bool hasNonListValue(const std::string& key) const;
    ssize_t pushList(const std::string& key, const std::string& value, bool front);
    ListResult popList(const std::string& key, std::string& value, bool front);

    std::mutex db_mutex;
    std::unordered_map<std::string, std::string> kv_store;
    std::unordered_map<std::string, std::vector<std::string>> list_store;
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> hash_store;

    std::unordered_map<std::string, std::chrono::steady_clock::time_point> expire_map;
};

#endif  // REDIS_DATABASE_HPP
