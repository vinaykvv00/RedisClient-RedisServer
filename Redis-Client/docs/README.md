# Client documentation

These documents describe the code as implemented, not an idealized Redis client.
Future behavior is explicitly marked as a recommendation or roadmap item.

## Reading paths

### Using the client

1. [Client README](../README.md): build, quick start, and feature boundaries.
2. [User guide](user-guide.md): interactive/one-shot operation and examples.
3. [Troubleshooting](troubleshooting.md): investigate unexpected results.

### Learning the implementation

1. [Architecture](architecture.md): responsibilities and execution paths.
2. [RESP protocol](resp-protocol.md): commands, responses, and TCP framing.
3. [Development and testing](development.md): validate behavior and extend it.

### Planning a dependable release

1. [Production-readiness assessment](production-readiness.md): prioritized gaps.
2. [Development test matrix](development.md#recommended-automated-tests): evidence
   needed before claiming reliability.

## Scope and vocabulary

- **Client:** this C++ executable. It sends commands and displays replies.
- **Server:** the separate process that owns data and executes Redis commands.
- **RESP:** the wire protocol between them, not Redis's internal storage format.
- **Implemented:** visible in the current source; not a claim of exhaustive tests.
- **Verified:** exercised by the checks explicitly listed in the development guide.
- **Recommended/planned:** not implemented merely because it appears in a diagram
  or checklist.

The sibling experimental server is not feature-complete. Server internals,
deployment, persistence, and full command compatibility are outside this client
documentation.

## Source map

| Source | Responsibility |
|---|---|
| [main.cpp](../main.cpp) | Client flags, remaining command arguments, program entry |
| [CLI.hpp](../include/CLI.hpp), [CLI.cpp](../src/CLI.cpp) | Session orchestration and terminal interaction |
| [CommandHandler.hpp](../include/CommandHandler.hpp), [CommandHandler.cpp](../src/CommandHandler.cpp) | Interactive tokenization and request encoding |
| [RedisClient.hpp](../include/RedisClient.hpp), [RedisClient.cpp](../src/RedisClient.cpp) | Connection, send loop, socket cleanup |
| [ResponseParser.hpp](../include/ResponseParser.hpp), [ResponseParser.cpp](../src/ResponseParser.cpp) | Reply parsing and display-string construction |
| [CMakeLists.txt](../CMakeLists.txt) | C++17 target, include path, compiler warning flags |

## External references

- [Official Redis RESP specification](https://redis.io/docs/latest/develop/reference/protocol-spec/)
- [Official Redis command reference](https://redis.io/docs/latest/commands/)
- [CMake documentation](https://cmake.org/documentation/)

External specifications define the target behavior; they do not establish that
this implementation supports every feature.
