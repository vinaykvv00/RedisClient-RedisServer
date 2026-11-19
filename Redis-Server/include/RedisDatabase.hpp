#ifndef REDIS_DATABASE_HPP
#define REDIS_DATABASE_HPP

#include <string>

class RedisDatabase {
public:
    // Add your database methods here
    static RedisDatabase& getInstance();

    //persistence Dump/load db from the file
    bool dump(const std::string& filename);
    bool load(const std::string& filename);
private:
    RedisDatabase() = default;
    ~RedisDatabase() = default;
    RedisDatabase(const RedisDatabase&) = delete;
    RedisDatabase& operator=(const RedisDatabase&) = delete;
};

#endif  // REDIS_DATABASE_HPP
