#include "../include/RedisCommandHandler.hpp"

#include <algorithm>
#include <charconv>
#include <iostream>
#include <sstream>
#include <vector>

#include "../include/RedisDatabase.hpp"

// RESP request format: an array of bulk strings.
//   *3\r\n$3\r\nSET\r\n$3\r\nkey\r\n$5\r\nvalue\r\n
//   *<number of elements>\r\n, then for each element: $<byte length>\r\n<bytes>\r\n

namespace {

// Reads a non-negative integer at `pos` that is terminated by "\r\n".
// On success stores it in `value`, moves `pos` past the "\r\n" and returns true.
bool readNumber(const std::string& input, size_t& pos, size_t& value) {
    size_t end = input.find("\r\n", pos);
    if (end == std::string::npos) return false;

    const char* first = input.data() + pos;
    const char* last = input.data() + end;
    auto result = std::from_chars(first, last, value);
    if (result.ec != std::errc() || result.ptr != last) return false;

    pos = end + 2;
    return true;
}

}  // namespace

// Splits a client command into tokens. Accepts RESP arrays and plain
// whitespace-separated text (e.g. "GET key" typed in telnet).
// Malformed RESP input yields the tokens parsed before the error.
std::vector<std::string> parseRespCommand(const std::string& input) {
    std::vector<std::string> tokens;

    if (input.empty() || input[0] != '*') {
        std::istringstream stream(input);
        std::string token;
        while (stream >> token) tokens.push_back(token);
        return tokens;
    }

    size_t pos = 1;  // skip '*'
    size_t count;
    if (!readNumber(input, pos, count)) return tokens;

    for (size_t i = 0; i < count; i++) {
        if (pos >= input.size() || input[pos] != '$') break;
        pos++;  // skip '$'

        size_t len;
        if (!readNumber(input, pos, len)) break;

        // The data must be followed by "\r\n"; written to avoid overflow.
        if (len > input.size() - pos || input.size() - pos - len < 2) break;
        if (input.compare(pos + len, 2, "\r\n") != 0) break;

        tokens.emplace_back(input, pos, len);
        pos += len + 2;
    }
    return tokens;
}

namespace {

using CommandArgs = std::vector<std::string>;

std::string wrongArgumentCount(const std::string& cmd) {
    return "-ERR wrong number of arguments for '" + cmd + "' command\r\n";
}

std::string handlePing(const CommandArgs& tokens) {
    if (tokens.size() != 1) return wrongArgumentCount("PING");
    return "+PONG\r\n";
}

std::string handleEcho(const CommandArgs& tokens) {
    if (tokens.size() != 2) return wrongArgumentCount("ECHO");
    return "+" + tokens[1] + "\r\n";
}

std::string handleFlushAll(RedisDatabase& db) {
    db.flushAll();
    return "+OK\r\n";
}

std::string handleSet(RedisDatabase& db, const CommandArgs& tokens) {
    if (tokens.size() != 3) return wrongArgumentCount("SET");
    db.set(tokens[1], tokens[2]);
    return "+OK\r\n";
}

std::string handleGet(RedisDatabase& db, const CommandArgs& tokens) {
    if (tokens.size() != 2) return wrongArgumentCount("GET");
    std::string value;
    if (!db.get(tokens[1], value)) return "$-1\r\n";

    std::ostringstream response;
    response << "$" << value.size() << "\r\n" << value << "\r\n";
    return response.str();
}

std::string handleKeys(RedisDatabase& db, const CommandArgs& tokens) {
    if (tokens.size() != 1) return wrongArgumentCount("KEYS");
    const auto allKeys = db.keys();
    std::ostringstream response;
    response << "*" << allKeys.size() << "\r\n";
    for (const auto& key : allKeys) {
        response << "$" << key.size() << "\r\n" << key << "\r\n";
    }
    return response.str();
}

std::string handleType(RedisDatabase& db, const CommandArgs& tokens) {
    if (tokens.size() != 2) return wrongArgumentCount("TYPE");
    return "+" + db.type(tokens[1]) + "\r\n";
}

std::string handleDel(RedisDatabase& db, const CommandArgs& tokens, const std::string& cmd) {
    if (tokens.size() != 2) return wrongArgumentCount(cmd);
    return db.del(tokens[1]) ? ":1\r\n" : ":0\r\n";
}

std::string handleExpire(RedisDatabase& db, const CommandArgs& tokens) {
    if (tokens.size() != 3) return wrongArgumentCount("EXPIRE");
    const int seconds = std::stoi(tokens[2]);
    return db.expire(tokens[1], seconds) ? ":1\r\n" : ":0\r\n";
}

std::string handleRename(RedisDatabase& db, const CommandArgs& tokens) {
    if (tokens.size() != 3) return wrongArgumentCount("RENAME");
    return db.rename(tokens[1], tokens[2]) ? "+OK\r\n" : "-ERR no such key\r\n";
}

constexpr auto wrongListType = "-WRONGTYPE Operation against a key holding the wrong kind of value\r\n";
constexpr auto invalidListInteger = "-ERR value is not an integer or out of range\r\n";
using ListResult = RedisDatabase::ListResult;

bool parseListInteger(const std::string& token, int& value) {
    const auto end = token.data() + token.size();
    const auto result = std::from_chars(token.data(), end, value);
    return result.ec == std::errc() && result.ptr == end;
}

std::string listIntegerReply(ssize_t result) {
    if (result < 0) return wrongListType;
    return ":" + std::to_string(result) + "\r\n";
}

std::string listValueReply(ListResult result, const std::string& value) {
    if (result == ListResult::WrongType) return wrongListType;
    if (result != ListResult::Ok) return "$-1\r\n";
    return "$" + std::to_string(value.size()) + "\r\n" + value + "\r\n";
}

std::string handleLLen(RedisDatabase& db, const CommandArgs& tokens) {
    if (tokens.size() != 2) return wrongArgumentCount("LLEN");
    return listIntegerReply(db.llen(tokens[1]));
}

std::string handleListPush(RedisDatabase& db, const CommandArgs& tokens, const std::string& cmd) {
    if (tokens.size() != 3) return wrongArgumentCount(cmd);
    const auto length = cmd == "LPUSH" ? db.lpush(tokens[1], tokens[2]) : db.rpush(tokens[1], tokens[2]);
    return listIntegerReply(length);
}

std::string handleListPop(RedisDatabase& db, const CommandArgs& tokens, const std::string& cmd) {
    if (tokens.size() != 2) return wrongArgumentCount(cmd);
    std::string value;
    const auto result = cmd == "LPOP" ? db.lpop(tokens[1], value) : db.rpop(tokens[1], value);
    return listValueReply(result, value);
}

std::string handleLRem(RedisDatabase& db, const CommandArgs& tokens) {
    if (tokens.size() != 4) return wrongArgumentCount("LREM");
    int count = 0;
    if (!parseListInteger(tokens[2], count)) return invalidListInteger;
    return listIntegerReply(db.lrem(tokens[1], count, tokens[3]));
}

std::string handleLIndex(RedisDatabase& db, const CommandArgs& tokens) {
    if (tokens.size() != 3) return wrongArgumentCount("LINDEX");
    int index = 0;
    if (!parseListInteger(tokens[2], index)) return invalidListInteger;
    std::string value;
    const auto result = db.lindex(tokens[1], index, value);
    return listValueReply(result, value);
}

std::string handleLSet(RedisDatabase& db, const CommandArgs& tokens) {
    if (tokens.size() != 4) return wrongArgumentCount("LSET");
    int index = 0;
    if (!parseListInteger(tokens[2], index)) return invalidListInteger;

    const auto result = db.lset(tokens[1], index, tokens[3]);
    if (result == ListResult::WrongType) return wrongListType;
    if (result == ListResult::Missing) return "-ERR no such key\r\n";
    if (result == ListResult::OutOfRange) return "-ERR index out of range\r\n";
    return "+OK\r\n";
}

}  // namespace

RedisCommandHandler::RedisCommandHandler() = default;

RedisCommandHandler::~RedisCommandHandler() = default;

std::string RedisCommandHandler::processCommand(const std::string& commandLine) {
    const auto tokens = parseRespCommand(commandLine);
    if (tokens.empty()) return "-ERR empty command\r\n";

    std::cout << "Received command: " << commandLine << std::endl;

    std::string cmd = tokens[0];
    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);
    RedisDatabase& db = RedisDatabase::getInstance();

    if (cmd == "PING") return handlePing(tokens);
    if (cmd == "ECHO") return handleEcho(tokens);
    if (cmd == "FLUSHALL") return handleFlushAll(db);
    if (cmd == "SET") return handleSet(db, tokens);
    if (cmd == "GET") return handleGet(db, tokens);
    if (cmd == "KEYS") return handleKeys(db, tokens);
    if (cmd == "TYPE") return handleType(db, tokens);
    if (cmd == "DEL" || cmd == "UNLINK") return handleDel(db, tokens, cmd);
    if (cmd == "EXPIRE") return handleExpire(db, tokens);
    if (cmd == "RENAME") return handleRename(db, tokens);
    if (cmd == "LLEN") return handleLLen(db, tokens);
    if (cmd == "LPUSH" || cmd == "RPUSH") return handleListPush(db, tokens, cmd);
    if (cmd == "LPOP" || cmd == "RPOP") return handleListPop(db, tokens, cmd);
    if (cmd == "LREM") return handleLRem(db, tokens);
    if (cmd == "LINDEX") return handleLIndex(db, tokens);
    if (cmd == "LSET") return handleLSet(db, tokens);
    return "-ERR unknown command '" + cmd + "'\r\n";
}
