# Development and testing

[Documentation index](README.md)

## Build configuration

[CMakeLists.txt](../CMakeLists.txt) declares:

- Minimum CMake version: 3.16.
- Language standard: C++17, required, with compiler extensions disabled.
- Target: `redis-client`.
- Sources: entry point and the four component implementations.
- Private include directory: `include/`.
- Compiler flags: `-Wall -Wextra`.
- Default build type: Debug for single-configuration generators.

No client test targets, CTest registration, install rules, package generation,
or CI workflow are currently provided.

From the repository root:

```bash
cmake -S Redis-Client -B Redis-Client/build
cmake --build Redis-Client/build
```

From the client directory, the equivalent is:

```bash
cmake -S . -B build
cmake --build build
```

CMake includes source files explicitly. When adding an implementation, add it
to `add_executable`; a header declaration alone does not provide linker symbols.
Do not edit generated files under `build/`.

## Manual end-to-end smoke check

### 1. Start an isolated server

With Redis installed, in a separate terminal:

```bash
redis-server --bind 127.0.0.1 --port 6380 --save "" --appendonly no
```

Use an unused port and a disposable instance, not a shared database. The examples
below assume `learn:client:missing` does not exist.

### 2. Run one-shot commands

From `Redis-Client/`:

```bash
./build/redis-client -p 6380 PING
./build/redis-client -p 6380 ECHO "hello world"
./build/redis-client -p 6380 ECHO ""
./build/redis-client -p 6380 SET learn:client:name foobar
./build/redis-client -p 6380 GET learn:client:name
./build/redis-client -p 6380 EXISTS learn:client:name
./build/redis-client -p 6380 GET learn:client:missing
./build/redis-client -p 6380 MGET learn:client:name learn:client:missing
./build/redis-client -p 6380 PINT
./build/redis-client -p 6380 DEL learn:client:name
```

Expected reply content, excluding connection banners:

| Command | Expected content |
|---|---|
| `PING` | `PONG` |
| `ECHO "hello world"` | `hello world` |
| `ECHO ""` | Empty line |
| `SET` | `OK` |
| `GET` of stored value | `foobar` |
| `EXISTS` | `1` |
| `GET` of missing key | `(nil)` |
| `MGET` | `1) foobar`, then `2) (nil)`; an extra blank line follows |
| `PINT` | Error beginning `(Error): ERR unknown command` |
| `DEL` | `1` |

Check reply text rather than `$?`: ordinary handled errors currently still
produce process status zero.

### 3. Check interactive mode

```bash
./build/redis-client -h localhost -p 6380
```

Try a blank line, `help`, `ECHO "hello world"`, and `quit`. Reopen it and try
end-of-input. Check that the prompt uses the configured host and port.

### 4. Clean up

Delete only the disposable keys you created. Stop the dedicated server with
`Ctrl+C`. Do not issue `FLUSHALL` against an existing/shared service.

## Verification performed for this documentation

On Linux, CMake 3.28.3 and GCC 13.3.0 successfully configured and built the
client. An isolated Redis process bound to loopback on a temporary port, with
persistence disabled, was used to verify:

- One-shot `PING`.
- `ECHO` with spaces and an empty argument.
- `SET`, `GET`, `EXISTS`, missing-key `GET`, `MGET`, and `DEL`.
- Unknown-command error display.
- Interactive blank input, placeholder help, quoted input, quit, and EOF.

The temporary server was stopped after checking. These checks were run ad hoc;
no persistent automated suite was added. This is not a compatibility
certification across Redis versions, operating systems, malformed input, or
network failure conditions.

## Recommended automated tests

The following matrix is a roadmap, not a claim of existing coverage.

| Layer | Cases | Required assertion |
|---|---|---|
| Option parsing | Defaults, custom endpoint, missing values, invalid/out-of-range/trailing-text port | Correct configuration or explicit usage failure |
| Command collection | `ECHO hello`, spaces, empty argument, `ECHO -p`, multiple arguments | Every argument preserved exactly once |
| Interactive tokenizer | Double quotes, empty quotes, whitespace, unmatched/escaped quotes | Documented token sequence or explicit error |
| Encoder | Empty values, CR/LF, UTF-8, embedded NUL at function level | Exact array count, byte lengths, payloads, CRLF |
| RESP simple types | Strings, errors, positive/negative integers | Correct content and, after redesign, preserved type |
| RESP bulk | Empty/null, binary bytes, split payload, split terminator | Exact payload and verified framing |
| RESP arrays | Empty/null, mixed elements, nested arrays | Correct recursive structure and bounded depth |
| Invalid replies | Invalid numbers, `< -1`, missing CRLF, EOF in every field | Explicit failure, no partial success, bounded work |
| Limits | Oversized lines/bulk/arrays and deeply nested data | Reject before excessive allocation/recursion |
| Transport | Short sends, interrupted system calls, reset/EOF, delayed response | No hangs, no accidental process death, explicit status |
| CLI integration | One-shot, REPL, help, EOF, connection failure | Correct output, lifecycle, and future exit codes |
| Sequential replies | Several replies in one write and replies split across writes | Exactly one complete reply consumed per parse |

### Suggested test approach

1. Start with pure encoding and tokenization tests.
2. Use POSIX `socketpair()` for exact response bytes without an external Redis.
3. Send reply fragments independently to simulate TCP fragmentation.
4. Use a disposable real Redis instance for end-to-end command semantics.
5. Add bounded timeouts to every network test.
6. Register tests with CTest and add CI only after a real suite exists.

Avoid timing-sensitive sleeps as the only readiness check. Probe server
readiness and always stop processes started by the test.

## Contribution checklist

- Keep changes focused on the intended behavior.
- Build with the configured warning flags.
- Add the smallest useful regression test for behavior changes.
- Preserve shell argument boundaries in one-shot mode.
- Preserve interactive behavior unless a change is intentional and documented.
- Do not swallow malformed input or socket errors as ordinary values.
- Add new implementation files to CMake.
- Keep secrets, server dumps, generated builds, and temporary logs out of Git.
- Update usage, protocol, and limitation documents together.

## Recommended implementation order

1. Input validation and process result propagation.
2. Complete RESP framing, length validation, and response limits.
3. Socket error/interrupt handling and finite timeouts.
4. Shared command execution path and clear error ownership.
5. Typed replies and a separate display renderer.
6. Expanded tests and CI.
7. Additional protocol/product features only when required.
