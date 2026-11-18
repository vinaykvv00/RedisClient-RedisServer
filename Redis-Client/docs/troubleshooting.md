# Troubleshooting

[Documentation index](README.md)

## Undefined reference to `ResponseParser::parseResponse`

**Meaning:** a declaration was visible while compiling, but the linker could not
find the matching implementation.

Check that:

1. [ResponseParser.cpp](../src/ResponseParser.cpp) defines the same signature as
   [ResponseParser.hpp](../include/ResponseParser.hpp).
2. The implementation is listed in [CMakeLists.txt](../CMakeLists.txt).
3. You rebuild the correct client build directory.

The current target includes `src/ResponseParser.cpp`. From `Redis-Client/`:

```bash
cmake -S . -B build
cmake --build build
```

Do not edit generated Makefiles or remove unrelated build directories.

## Failed to connect to `localhost:1111`

`-p 1111` changes the client's destination, not the server's listening port.
A server must already be accepting connections there.

If your Redis server uses its default port:

```bash
./build/redis-client -h localhost -p 6379
```

For a dedicated learning server on another unused port:

```bash
redis-server --bind 127.0.0.1 --port 6380 --save "" --appendonly no
```

Then connect from another terminal:

```bash
./build/redis-client -h 127.0.0.1 -p 6380 PING
```

If available, inspect listening TCP sockets with:

```bash
ss -ltn
```

Check the host, port, server logs, and address-family binding. For a remote
server, also check routing and firewall rules rather than disabling protection.
The networking layer and CLI currently both print a connection-failure message;
two lines can describe the same failure.

## Unknown command when entering `$6\r\nfoobar\r\n`

The prompt accepts Redis commands, not raw protocol text. Literal `\r\n`
characters typed at the prompt are not decoded as network CR/LF bytes.

Try:

```text
SET learn:client:name foobar
GET learn:client:name
DEL learn:client:name
```

The server's reply to `GET` is a bulk string; the client generates/parses the
wire framing for you. `OK` is normally a status reply, not a command.

## `ECHO "hello"` reports the wrong number of arguments

Check the command-collection loop in [main.cpp](../main.cpp). Each argument must
advance the index exactly once:

```cpp
while (i < argc) {
    commandArgs.push_back(argv[i++]);
}
```

An additional `i++` inside that loop skips arguments. The current implementation
uses only one increment. Rebuild and run the executable from the corresponding
build directory.

## The prompt shows the wrong host or port

[CLI.cpp](../src/CLI.cpp) initializes its `host_` and `port_` members from
constructor parameters and uses them in the prompt. Do not introduce hardcoded
local endpoint variables inside `run()`.

The prompt shows the configured hostname, not necessarily the numeric address
selected by DNS resolution.

## Invalid port crashes or behaves unexpectedly

Port parsing currently uses `std::stoi` without complete validation or exception
handling. Supply a numeric port in `1..65535`; do not rely on malformed input
being rejected cleanly.

This is a known implementation gap, not correct production error handling.

## Client waits forever

Possible causes:

- A server accepted the connection but has not sent a complete reply.
- A blocking Redis command is legitimately waiting.
- A reply was truncated or protocol framing was lost.
- An interactive send failed but the code still attempted to parse.
- The experimental sibling server did not generate a reply.

The client has no configured I/O deadlines. Interrupt a stuck learning session
with `Ctrl+C`, inspect the server and command, and reconnect if framing may be
lost. Do not automatically replay writes after an uncertain result.

## `(Error): Unknown response prefix`

The parser only recognizes RESP2 prefixes `+`, `-`, `:`, `$`, and `*`.
Possible causes include RESP3 negotiation, connecting to a non-Redis service,
or desynchronized framing.

Do not send `HELLO 3`. Restart the client after an unsupported/malformed response;
the current parser does not resynchronize the stream.

## Array output has extra blank lines or unclear nesting

The parser renders array elements with trailing newlines; the CLI adds another
newline. Nested arrays are formatted recursively without a structured indentation
model. This is a known output limitation.

## A script reports success after a connection or Redis error

`main()` normally returns zero after `run()` returns, regardless of those handled
failures. Exit-status propagation is not implemented.

Do not use this client for automation that depends on trustworthy success/failure
statuses until that contract is implemented and tested.

## Commands work against Redis but not the sibling server

The [experimental command handler](../../Redis-Server/src/RedisCommandHandler.cpp)
parses requests but does not yet execute commands or produce normal database
responses. Use an actual Redis server for client compatibility testing.

## Reporting a problem

Include:

- Operating system, compiler, CMake version, and Redis version.
- Exact build command and client invocation.
- Expected and actual reply/output.
- Whether the failure occurs in one-shot mode, interactive mode, or both.
- Whether the server is Redis or the experimental sibling project.
- A minimal reproducible example using disposable keys.

Remove credentials, private hostnames, and sensitive values from shared logs.
