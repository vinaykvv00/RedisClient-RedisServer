# User guide

[Documentation index](README.md) | [Build instructions](../README.md#quick-start)

## Intended use

Use this client to learn basic Redis request/reply communication on a trusted
local development machine. It is not intended for unattended production
automation, untrusted servers, or public-network use.

The examples assume you have built the executable and your current directory is
`Redis-Client/`. They use a disposable Redis instance on port `6380`, started as
described in the [quick start](../README.md#start-a-local-learning-server).

## Command syntax

```text
./build/redis-client [-h HOST] [-p PORT] [COMMAND [ARG ...]]
```

| Input | Meaning |
|---|---|
| `-h HOST` | Server host; default `127.0.0.1` |
| `-p PORT` | Server port; default `6379` |
| No command | Start an interactive session |
| Command and arguments | Execute one command, display one reply, and exit |

Hostnames are resolved with `getaddrinfo`; the client tries available resolved
addresses, allowing IPv4 or IPv6 where supported by the host and server.

**Options must precede the command.** After the first non-option argument,
everything remaining is command data:

```bash
./build/redis-client -p 6380 ECHO "-p"
```

This sends `ECHO` with the value `-p`; it does not look for another port.

There is no `--help`, URI configuration, password flag, database-selection flag,
environment-based configuration, or configuration file. Missing option values
and invalid ports are not cleanly validated. Supply a valid numeric port in
`1..65535`.

## Interactive mode

```bash
./build/redis-client -h localhost -p 6380
```

Example session, after the connection banner:

```text
localhost:6380> SET learn:client:greeting "hello world"
OK
localhost:6380> GET learn:client:greeting
hello world
localhost:6380> EXISTS learn:client:greeting
1
localhost:6380> DEL learn:client:greeting
1
localhost:6380> GET learn:client:greeting
(nil)
localhost:6380> quit
Exiting... goodbye
```

Local controls are case-sensitive:

| Input | Current behavior |
|---|---|
| Blank/whitespace-only line | Ignore and show another prompt |
| `help` | Print `display help information` with a leading space; detailed help is not implemented |
| `quit` or `exit` | Leave the client without sending that line to Redis |
| End-of-input, typically `Ctrl+D` on an empty terminal line | Leave the read loop |

Other inputs are tokenized and sent. Redis normally treats command names
case-insensitively, but the client's local controls are lowercase exact matches.

### Interactive quoting

Supported common case:

```text
ECHO "hello world"
```

The double-quoted portion becomes one argument, without the enclosing quotes.
An empty double-quoted argument (`ECHO ""`) is also recognized.

The tokenizer is a regular expression, not a shell. Do not assume support for
single-quote grouping, escaped quotes, backslash decoding, adjacent quoted-word
concatenation, or errors for unmatched quotes. For example, typing `\r\n` does
not produce CR/LF bytes.

## One-shot mode

```bash
./build/redis-client -p 6380 PING
./build/redis-client -p 6380 ECHO "hello world"
./build/redis-client -p 6380 ECHO ""
```

The shell performs argument grouping before the client starts. The client
preserves each resulting argument; it does not run the interactive tokenizer
again. In a POSIX shell, either single or double quotes can group a shell
argument, according to shell rules.

Do not enter the interactive prompt text as part of the command.

## Trying each main RESP2 reply

Use only a disposable database. The commands below write/delete keys prefixed
with `learn:client:`:

| Interactive command | Expected result | Reply category |
|---|---|---|
| `PING` | `PONG` | Simple string |
| `SET learn:client:name foobar` | `OK` | Simple string |
| `GET learn:client:name` | `foobar` | Bulk string |
| `EXISTS learn:client:name` | `1` | Integer |
| `DEL learn:client:missing` | `0`, assuming absent | Integer |
| `GET learn:client:missing` | `(nil)`, assuming absent | Null bulk string |
| `MGET learn:client:name learn:client:missing` | Numbered values `foobar` and `(nil)` | Array |
| `PINT` | `(Error): ERR unknown command ...` | Simple error |
| `DEL learn:client:name` | `1` after the earlier SET | Integer |

The client does not implement these database operations itself. The connected
server must support them.

## Understanding output

- Startup prints `Connected to Redis server.` and `Socket FD: N` to stdout.
- Simple strings, integers, and bulk strings are printed as plain text.
- Server errors are prefixed with `(Error):`.
- Null bulk strings and null arrays both display `(nil)`.
- Array elements are numbered starting at `1)`. Array formatting already ends
  with a newline; the CLI adds another, so a blank line can follow.
- Empty bulk strings and empty arrays both appear as an empty result line.
- Nested arrays are parsed recursively but are not clearly indented or typed.

Output is for human learning, not lossless machine interchange. A literal value
`(nil)` looks like null, and an error-looking string can resemble an actual error.

## Authentication and stateful commands

There is no automatic login or TLS. In a trusted local interactive session,
commands such as `AUTH` and `SELECT` can be sent manually if required by the
server. State applies only to that session's TCP connection.

Running a one-shot `SELECT` does not select a database for a later invocation:
the later process opens a new connection. Avoid entering credentials on command
lines; they can be exposed through shell history and process listings.

## Unsupported workflows

- RESP3 negotiation (`HELLO 3`).
- Pub/Sub consumption and unsolicited push messages.
- A dedicated pipeline API or asynchronous/multiplexed requests.
- Redis Cluster routing, `MOVED`/`ASK` handling, Sentinel discovery.
- TLS, automatic authentication, automatic reconnect/retry.
- Raw-response, JSON, CSV, or stable machine-readable output modes.
- Arbitrary binary input via a stdin/file option. The encoder uses byte lengths,
  but shell arguments cannot contain NUL bytes and the REPL is line-oriented.

Do not send commands that switch into an unsupported protocol or reply mode.

## Exit behavior and failures

`main()` currently returns `0` after `CLI::run()` returns, including ordinary
connection-failure paths and displayed Redis errors. Uncaught conversion
exceptions or operating-system signals can terminate differently.

Consequently, shell code such as `if ./build/redis-client ...; then ...; fi`
cannot reliably distinguish a successful Redis command from a handled failure.
Use the [troubleshooting guide](troubleshooting.md) to investigate manually.
