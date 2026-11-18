# RESP2 implementation guide

[Documentation index](README.md) |
[Official protocol specification](https://redis.io/docs/latest/develop/reference/protocol-spec/)

## Commands versus wire bytes

The user enters `GET name`. The client sends an array containing two bulk strings:

```text
*2\r\n$3\r\nGET\r\n$4\r\nname\r\n
```

In this document, `\r\n` represents the actual two bytes CR (`0x0d`) and LF
(`0x0a`). Typing backslash-r-backslash-n in the REPL does not create those bytes.

Lengths are **byte counts**, not character counts. UTF-8 characters may occupy
multiple bytes. `std::string::size()` supplies the byte length for the encoder.

## Request encoding

For arguments `["SET", "name", "foobar"]`:

```text
*3\r\n
$3\r\nSET\r\n
$4\r\nname\r\n
$6\r\nfoobar\r\n
```

These lines illustrate segments of one message; do not insert additional
newlines between them. The encoded stream is:

```text
*3\r\n$3\r\nSET\r\n$4\r\nname\r\n$6\r\nfoobar\r\n
```

[CommandHandler::buildRESPCommand](../src/CommandHandler.cpp) writes:

1. `*`, the number of arguments, then CRLF.
2. For each argument: `$`, its byte length, CRLF, its exact bytes, and CRLF.

RESP bulk strings can contain whitespace, CR/LF, and NUL bytes. The encoding
function uses string lengths rather than C-string termination, but the current
command-line interfaces do not provide a general arbitrary-binary input facility.

## Implemented reply forms

The five core RESP2 types are simple strings, simple errors, integers, bulk
strings, and arrays. Null bulk strings and null arrays are special encodings.

| Reply | Wire example | Current displayed result |
|---|---|---|
| Simple string | `+OK\r\n` | `OK` |
| Simple error | `-ERR example\r\n` | `(Error): ERR example` |
| Integer | `:42\r\n` | `42` |
| Bulk string | `$6\r\nfoobar\r\n` | `foobar` |
| Empty bulk string | `$0\r\n\r\n` | Empty text |
| Null bulk string | `$-1\r\n` | `(nil)` |
| Array | `*2\r\n$3\r\nfoo\r\n$3\r\nbar\r\n` | `1) foo`, then `2) bar` |
| Empty array | `*0\r\n` | Empty text |
| Null array | `*-1\r\n` | `(nil)` |

### Dispatch and simple replies

[ResponseParser::parseResponse](../src/ResponseParser.cpp) reads the first byte
and dispatches by `+`, `-`, `:`, `$`, or `*`. Unknown prefixes produce an
error-looking display string, but the remaining reply is not consumed and the
connection is not automatically closed.

Simple strings, errors, and integers use a CRLF-oriented line helper.
Integer contents are returned as text, not validated or stored as integers.

### Bulk-string replies

The parser reads the length line with `std::stoi`. For `-1` it returns `(nil)`;
otherwise it resizes a string, loops until the declared payload length has been
received, and attempts two additional reads for trailing CRLF.

The payload loop correctly avoids treating a newline *inside* the payload as
its end. However, the two terminator reads are not checked for success or for
the actual CR/LF values.

### Array replies

The parser reads an element count and recursively calls `parseResponse()` for
each element, so it is not restricted to arrays of bulk strings. It formats each
element with a numbered prefix.

Recursive parsing is present, but there is no recursion-depth limit. An element
failure is not propagated as a structured failure of the entire array.

## TCP framing: the most important rule

**TCP is a byte stream, not a message transport.**

```mermaid
flowchart LR
    Wire["One RESP bulk string"] --> A["Read 1: prefix and part of length"]
    A --> B["Read 2: rest of length and some payload"]
    B --> C["Read 3: remaining payload and CRLF"]
```

Likewise, a single socket read may contain bytes from multiple messages. Never
assume one `send()` matches one `recv()`.

Current code reads prefix/line characters one byte at a time and bulk payloads
in a loop. This can handle many valid fragmented replies, but it does not make
the parser complete or robust.

## Required correctness improvements

Before treating RESP2 support as reliable:

1. Require complete CRLF lines and reject malformed or truncated lines.
2. Validate numeric fields completely; reject overflow, invalid trailing text,
   and lengths less than `-1`.
3. Set maximum line length, bulk length, array count, nesting depth, and total
   response budget before allocating or recursing.
4. Read and verify exact payload terminators.
5. Distinguish EOF, interrupted reads, socket errors, server error replies, and
   protocol errors.
6. Stop processing or close a connection after framing is lost; do not continue
   reading an unknown remainder as if it were the next reply.
7. Preserve typed values separately from human-readable display output.

The line helper currently can return partial data at EOF, and malformed CR/LF
can be mishandled rather than rejected. A negative length other than `-1` or an
oversized length can cause invalid allocation behavior or exceptions.

## RESP3 and out-of-scope modes

RESP3 types such as booleans, doubles, maps, sets, attributes, and pushes are
not supported. Do not send `HELLO 3` and then expect this parser to work.

RESP2 Pub/Sub also needs a continuous message-consumption workflow beyond the
current one-command/one-reply loop. Adding only another prefix case would not
implement a complete subscription or push-message interface.

## Useful protocol tests

Use `socketpair()` or an isolated fake server to supply exact bytes:

```text
+OK\r\n
-ERR example\r\n
:42\r\n
$0\r\n\r\n
$-1\r\n
*0\r\n
*-1\r\n
*2\r\n$3\r\nfoo\r\n:2\r\n
```

Also test embedded CR/LF in bulk payloads, nested arrays, split CR/LF pairs,
partial payloads, invalid lengths, wrong terminators, and consecutive replies.
See the [test matrix](development.md#recommended-automated-tests).
