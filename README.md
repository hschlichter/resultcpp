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

### `ok_or`

Convert a nullable pointer into `Result<Ptr,E>`:

```cpp
template<typename Ptr, typename E>
auto ok_or(Ptr ptr, E err) -> Result<decltype(ptr), E>;
```

Usage:

```cpp
SDL_Window* w = SDL_CreateWindow(...);
auto res = ok_or(w, SdlError::WindowCreate);
```

### `.map_err(...)`

Transform the error type without modifying the success value:

```cpp
result.map_err(AppErrorTraits<SdlError>::convert);
```

---

## Quickstart Example

```cpp
#include "result.hpp"

...

int main() {
  // 1. Initialize SDL
  unwrap_or_exit(
    init_sdl()
      .map_err(AppErrorTraits<SdlError>::convert)
  );

  // 2. Create window
  SDL_Window* window = unwrap_or_exit(
    create_window()
      .map_err(AppErrorTraits<SdlError>::convert)
  );

  // 3. Create Vulkan instance
  VkInstance instance = unwrap_or_exit(
    create_instance(window)
      .map_err(AppErrorTraits<VkError>::convert)
  );

  // 4. Create Vulkan surface
  VkSurfaceKHR surface = unwrap_or_exit(
    create_surface(window, instance)
      .map_err(AppErrorTraits<SdlError>::convert)
  );

  printf("Initialization successful.\n");
  return 0;
}
```

Each call returns a `Result<T,DomainError>`, which is then mapped into an `AppError` and unwrapped or exits on failure.

---

## Helpers & Extensions

- **`unwrap_or_exit(r)`**: If `r` is `Err`, prints the error and calls `std::exit(1)`; otherwise returns the contained value.  
- **Additional patterns** you can add:
  - `.map(F f)` — transform the success value.  
  - `.and_then(F f)` — chain another fallible call.  
  - `.unwrap_or(default)` — return the value or a default.

Feel free to extend `result.hpp` with these utilities as needed.

---

## Defining New Error Domains

1. **Declare** your error enum:
```cpp
    enum class MyError { Foo, Bar, Baz };
```
2. **Specialize** the conversion traits:
```cpp
    template<> struct AppErrorTraits<MyError> {
        static AppError convert(MyError e) {
            return AppError{ AppError::Kind::MyDomain, .data = { .my = e } };
        }
    };
```
3. **Return** `Result<T,MyError>` from your functions.



