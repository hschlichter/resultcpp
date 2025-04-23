# Result<T, E> Error Handling for C++

A lightweight, Rust‑inspired `Result<T, E>` utility for C++ projects using C‑style APIs. 
Eliminates nested checks, clarifies error domains, and keeps your “happy path” linear.

---

## Features

- **Explicit**: Every fallible function returns `Result<T, E>`, making errors part of the signature.
- **Type‑Safe**: Separate error enums for SDL (`SdlError`) and Vulkan (`VkError`), with conversion to a unified `AppError`.
- **Zero Overhead**: A tag + union adds only a couple of bytes—no exceptions, no RTTI.
- **Composable**: Chain operations with `.map_err()`, `ok_or()`, and custom traits to maintain linear control flow.
- **Customizable**: Easily add new domains or helpers (`map()`, `and_then()`, `unwrap_or(default)`, etc.).

---

## Installation

1. **Copy** the header file (`result.hpp`) into your project’s `include/` directory.  
2. **Include** them in your source files:
   ```cpp
   #include "result.hpp"
   ```
3. **Compile** with any C++11‑compatible (or newer) compiler—no special flags required.

---

## Core Concepts

### `Result<T, E>`

A tagged union representing either a success (`Ok`) or an error (`Err`):

```cpp
template<typename T, typename E>
struct Result {
    enum class Tag { Ok, Err } tag;
    union { T value; E error; };

    template<typename F>
    auto map_err(F f) const -> Result<T, decltype(f(std::declval<E>()))>;
};
```

- **`ok<T,E>(val)`** constructs a successful result.  
- **`err<T,E>(err)`** constructs an error result.

---

## Example

```cpp
#include "result.hpp"

...
enum class ParseError {
    Empty, 
    NotANumber 
};

template<>
struct Display<ParseError> {
    static void print(ParseError e) {
        switch(e) {
            case ParseError::Empty: std::cerr << "Error: input was empty\n"; break;
            case ParseError::NotANumber: std::cerr << "Error: not a number\n"; break;
        }
    }
};

// Try to parse an integer; empty string or non‑digits produce errors
Result<int,ParseError> parse_int(const std::string& s) {
    if (s.empty()) {
        return err<int,ParseError>(ParseError::Empty);
    }

    for (char c : s) {
        if (!isdigit(c)) {
            return err<int, ParseError>(ParseError::NotANumber);
        }
    }

    return ok<int, ParseError>(std::stoi(s));
}

// Validate that the number is positive
Result<void, ParseError> validate_positive(int x) {
    if (x <= 0) {
        return err<void, ParseError>(ParseError::NotANumber);
    }

    return ok<ParseError>();
}

int main() {
    // Fatal: must parse or exit
    int n = unwrap(parse_int("123"));
    assert(n == 123);

    // Non‑fatal: fallback to zero
    int m = unwrap_or(parse_int(""), 0);
    assert(m == 0);

    // Non‑fatal with custom handler
    int k = unwrap_or_else(parse_int("abc"), [](ParseError){
        return 42;
    });
    assert(k == 42);

    // Void unwrap
    unwrap(validate_positive(n));

    // Void non‑fatal: just prints if error
    auto result = validate_positive(-5);
    assert((result.tag == Result<void, ParseError>::Tag::Err));

    // Void non‑fatal with handler
    bool cleaned = false;
    unwrap_or_else(validate_positive(-1), [&](ParseError) {
        cleaned = true;
    });
    assert(cleaned);

    return 0;
}
```

