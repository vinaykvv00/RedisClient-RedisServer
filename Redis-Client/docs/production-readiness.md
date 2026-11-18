# Code assessment and production-readiness roadmap

[Documentation index](README.md)

## Overall assessment

The client has a sensible small-project structure and the essential happy-path
features for learning Redis. It is appropriate for local experiments with a
trusted Redis server. **It is not currently production-ready.**

Professional documentation does not change that status. This assessment is based
on the current source and the smoke checks described in the
[development guide](development.md), not a full reliability or security
certification.

## What is already in place

| Basic capability | Current implementation |
|---|---|
| Endpoint configuration | `-h` / `-p`, with localhost defaults |
| Connection establishment | Address resolution and attempts across available addresses |
| Socket cleanup | Destructor closes an open descriptor |
| Argument preservation | One-shot mode keeps shell-provided argument boundaries |
| Interactive command input | Basic tokenization including double-quoted values |
| Command encoding | RESP arrays containing byte-counted bulk strings |
| Short-write handling | Send loop advances until the command is sent |
| Main RESP2 reply forms | Prefix dispatch, bulk payload loop, recursive arrays |
| User modes | Interactive REPL and one-shot command execution |

These are good foundations. Basic correctness still requires the improvements
below.

## Priority 1: make failures explicit and framing dependable

### Validate process arguments

Source: [main.cpp](../main.cpp).

- Missing `-h`/`-p` values currently fall through into command collection.
- `std::stoi` exceptions are not handled.
- Port range and full consumption are not checked; text with a numeric prefix
  can be partially accepted.
- Unknown flags can be interpreted as command names rather than usage errors.

**Target:** define a documented option grammar, reject invalid input with a
clear usage message, and return a nonzero process status.

### Propagate command outcomes to the process

Sources: [main.cpp](../main.cpp), [CLI.cpp](../src/CLI.cpp).

`run()` and `executeCommand()` return `void`; `main()` normally returns zero
regardless of displayed command errors or connection failure.

**Target:** distinguish successful replies, Redis errors, transport errors,
protocol errors, and usage errors; return documented statuses without treating
an error-looking string as the sole error channel.

### Enforce complete RESP framing

Source: [ResponseParser.cpp](../src/ResponseParser.cpp).

- Line reads can return incomplete data at EOF.
- CR/LF sequences are not strictly validated.
- Bulk terminator reads are unchecked.
- Length conversion can throw or accept malformed numeric suffixes.
- Lengths below `-1` are not rejected consistently.
- Unsupported prefixes leave unread bytes without a resynchronization policy.

**Target:** every field must be complete and valid. On lost framing, stop/close
the session rather than treating remaining bytes as a new reply.

### Bound parser resource use

Source: [ResponseParser.cpp](../src/ResponseParser.cpp).

No maximum line length, bulk length, array count, nesting depth, or total reply
budget is enforced.

**Target:** check configurable or documented finite limits before allocations,
loops, and recursion. Return explicit errors when limits are exceeded.

### Stop after send failure

Source: [CLI.cpp](../src/CLI.cpp).

The one-shot helper stops on send failure, but the interactive loop prints an
error and still calls the reply parser. That can wait for a reply to a command
that was not delivered.

**Target:** use a shared execution path and decide explicitly whether a broken
connection ends the session or requires user-directed reconnect.

## Priority 2: make transport behavior predictable

Source: [RedisClient.cpp](../src/RedisClient.cpp), plus the parser's socket reads.

| Gap | Recommended outcome |
|---|---|
| Blocking connect/read/write with no configured timeout | Finite deadlines and actionable timeout errors |
| No interrupted-system-call policy | Retry `EINTR` where appropriate without hiding genuine failures |
| `send()` does not suppress/handle `SIGPIPE` | A closed peer becomes an explicit transport failure rather than unexpected process termination |
| Zero-byte send has no progress policy | Avoid a potential no-progress loop |
| EOF and read failure collapse into similar paths | Distinct diagnostic outcomes |
| Public repeated connection attempts lack an explicit lifecycle contract | Close/reuse existing state deliberately |
| Socket-owning type is implicitly copyable | Delete copying; define safe moves only if needed |

Do not blindly retry commands after reconnect. A write command might have
executed before the reply was lost; replaying it can change data twice.

## Priority 3: improve maintainability and user experience

- Separate parsed reply values from rendering so nulls, strings, errors, and
  nested arrays retain their identity.
- Consolidate one-shot and interactive command execution.
- Choose one layer to own each diagnostic; connection failures currently print
  twice.
- Move connection banners/debug details away from result stdout, with a
  documented compatibility decision.
- Replace placeholder help with real usage and limitations.
- Define quote/escape rules and reject malformed interactive input explicitly.
- Consider line history and completion only after correctness.
- Prefer a bounded buffered socket reader over one syscall per line character
  when optimizing; benchmark before making performance claims.

These are proposed changes, not implemented behavior.

## Priority 4: add operational features only when required

| Requirement | Current status | Future work |
|---|---|---|
| Encrypted transport | No TLS | Add certificate-verified TLS with documented trust configuration |
| Automatic authentication | No startup login workflow | Define credential input that avoids history/process-list exposure |
| Database selection | Manual per-session `SELECT` only | Optional startup selection and state handling |
| Cluster/Sentinel | No discovery or routing | Implement topology and redirect semantics if needed |
| Pub/Sub | No continuous subscription consumer | Separate streaming lifecycle and output model |
| RESP3 | No parser or negotiation support | Typed values, negotiation, pushes, expanded protocol tests |
| Distribution | No install/package/release pipeline | Packaging, supported-platform matrix, release process |

Typing `AUTH` or `SELECT` as ordinary commands does not implement a full
configuration or credential-management feature.

## Suggested release gates

This is an acceptance checklist for a future dependable basic client:

- [ ] Validated CLI options with helpful usage errors.
- [ ] Reliable documented process statuses.
- [ ] Strict, bounded RESP2 parsing including null and nested values.
- [ ] Explicit handling of EOF, truncated data, interruption, reset, and timeout.
- [ ] No hang after failed sends; no unintended `SIGPIPE` termination.
- [ ] Non-copyable or safely movable socket ownership.
- [ ] Automated unit/integration tests for the development matrix.
- [ ] CI builds and tests on every claimed platform/compiler.
- [ ] Stable documented output contract for supported modes.
- [ ] A declared license, supported environments, and release/version policy.
- [ ] TLS/authentication before any deployment scenario that requires them.

Passing these checks would still not make the client a complete replacement for
official Redis tooling. Feature scope must remain explicit.

## What not to build yet

For this learning project, prioritize reliable RESP2 request/reply behavior over
RESP3, asynchronous frameworks, connection pools, cluster support, or large
abstraction layers. A small, well-tested synchronous client is a useful and
complete learning milestone.
