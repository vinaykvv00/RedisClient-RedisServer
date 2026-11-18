# Redis Client and Server in C++

A learning repository for understanding Redis-style networking, command-line
interfaces, and the Redis Serialization Protocol (RESP).

**Status:** educational work in progress, not a production-ready Redis replacement.
This project is independent of Redis.

## Projects

| Project | Current scope | Where to start |
|---|---|---|
| [Redis client](Redis-Client/README.md) | C++17 TCP client with interactive and one-shot modes, RESP command encoding, and basic RESP2 reply parsing | [Build and quick start](Redis-Client/README.md#quick-start) |
| [Redis server](Redis-Server/) | Experimental server; its command handler parses input but command dispatch and database execution are unfinished | [Server entry point](Redis-Server/src/main.cpp), [command handler](Redis-Server/src/RedisCommandHandler.cpp) |

Use an existing Redis server to exercise the client examples. Do not assume the
sibling server implements `PING`, `SET`, or `GET`: its current command handler
does not generate those command results. The background persistence thread is
also a placeholder, not implemented persistence.

## Build and try the client

Requirements: Linux or a compatible POSIX environment, a C++17 compiler,
CMake 3.16+, and a build tool such as Make or Ninja. WSL users should build
inside their Linux environment.

From the repository root:

```bash
cmake -S Redis-Client -B Redis-Client/build
cmake --build Redis-Client/build
```

With a Redis server already listening on `127.0.0.1:6379`:

```bash
# One command, then exit
./Redis-Client/build/redis-client PING
./Redis-Client/build/redis-client ECHO "hello world"

# Interactive session
./Redis-Client/build/redis-client
```

The client uses a server; it does not start one. It currently prints connection
information as well as command results.

## Documentation

- [Client overview and feature status](Redis-Client/README.md)
- [Documentation index and suggested reading order](Redis-Client/docs/README.md)
- [User guide](Redis-Client/docs/user-guide.md)
- [Architecture and diagrams](Redis-Client/docs/architecture.md)
- [RESP2 implementation guide](Redis-Client/docs/resp-protocol.md)
- [Development and testing](Redis-Client/docs/development.md)
- [Code assessment and production-readiness roadmap](Redis-Client/docs/production-readiness.md)
- [Troubleshooting](Redis-Client/docs/troubleshooting.md)

The detailed documentation currently covers the client, not the entire server.

## Contributing and project boundaries

Keep changes focused, preserve interactive and one-shot behavior, and document
protocol or output changes. See the [development guide](Redis-Client/docs/development.md)
for a proposed test matrix and contribution checklist.

Do not use the examples against a production database. Use an isolated local
Redis instance and disposable keys.

## License

No license file is currently provided in this repository. Do not assume a
particular open-source license; the maintainer must choose one before publishing
license-based usage or redistribution terms.
