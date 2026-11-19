#include "../include/RedisDatabase.hpp"

RedisDatabase& RedisDatabase::getInstance() {
    static RedisDatabase instance;
    return instance;
}

bool RedisDatabase::dump(const std::string& filename) {
    return true;
}
bool RedisDatabase::load(const std::string& filename) {
    return true;
}
