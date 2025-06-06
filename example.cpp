#include <cassert>
#include <iostream>
#include <string>
#include "result.hpp"

enum class RootError {
    Hello, 
};

template<>
struct Display<RootError> {
    static void print(RootError e) {
        switch(e) {
            case RootError::Hello: std::cerr << "hello\n"; break;
        }
    }
};

enum class NestedError {
    World, 
};

template<>
struct Display<NestedError> {
    static void print(NestedError e) {
        switch(e) {
            case NestedError::World: std::cerr << "world\n"; break;
        }
    }
};


auto world() -> Result<int, NestedError> {
    return ok<int, NestedError>(0);
}

auto hello() -> Result<int, RootError> {
    return world().map_err([] (NestedError) { return RootError::Hello; });
}

auto test_nested_error() {
    unwrap(hello());
}

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
Result<int, ParseError> parse_int(const std::string& s) {
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
    int k = unwrap_or_else(parse_int("abc"), [] (ParseError){
        return 42;
    });
    assert(k == 42);

    // Void unwrap
    validate_positive(n).unwrap();

    // Void non‑fatal: just prints if error
    auto result = validate_positive(-5);
    assert((result.tag == Result<void, ParseError>::Tag::Err));

    // Void non‑fatal with handler
    bool cleaned = false;
    unwrap_or_else(validate_positive(-1), [&](ParseError) {
        cleaned = true;
    });
    assert(cleaned);

    // Map value into different type
    auto c = parse_int("42")
        .map([] (int i) -> char {
            return static_cast<char>(i);
        })
        .unwrap();
    assert(c == '*');

    // Linear execution
    auto check = false;
    parse_int("234")
        .and_then([&] (int) {
            check = true;
            return ok<ParseError>();
        });
    assert(check);

    // Match result
    auto check_flow = false;
    auto i = parse_int("1234");
    match(
        i,
        [&] (int ii) {
            assert(ii == 1234);
            check_flow = true;
        },
        [] (ParseError) {
            assert(false);
        }
    );
    assert(check_flow);

    test_nested_error();
    return 0;
}
