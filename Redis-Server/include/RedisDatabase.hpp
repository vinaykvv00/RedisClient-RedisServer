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
};

#endif  // REDIS_DATABASE_HPP
