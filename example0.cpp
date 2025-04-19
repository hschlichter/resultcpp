#include <iostream>
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
    return Ok<int, NestedError>(0);
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
Result<int,ParseError> parse_int(const std::string& s) {
    if (s.empty()) {
        return Err<int,ParseError>(ParseError::Empty);
    }

    for (char c : s) {
        if (!isdigit(c)) {
            return Err<int, ParseError>(ParseError::NotANumber);
        }
    }

    return Ok<int, ParseError>(std::stoi(s));
}

// Validate that the number is positive
Result<void, ParseError> validate_positive(int x) {
    if (x <= 0) {
        return Err<void, ParseError>(ParseError::NotANumber);
    }

    return Ok<ParseError>();
}

int main() {
    // Fatal: must parse or exit
    int n = unwrap(parse_int("123"));
    std::cout << "Parsed: " << n << "\n";

    // Non‑fatal: fallback to zero
    int m = unwrap_or(parse_int(""), 0);
    std::cout << "Got m=" << m << "\n";

    // Non‑fatal with custom handler
    int k = unwrap_or_else(parse_int("abc"), [](ParseError){
        std::cout << "Recovering from bad input, using 42\n";
        return 42;
    });
    std::cout << "Got k=" << k << "\n";

    // Void unwrap
    unwrap(validate_positive(n));
    std::cout << "n is positive\n";

    // Void non‑fatal: just prints if error
    unwrap_or(validate_positive(-5), (void*)nullptr);
    std::cout << "Continuing after validation\n";

    // Void non‑fatal with handler
    unwrap_or_else(validate_positive(-1), [&](ParseError){
        std::cout << "Cleanup on invalid input\n";
    });

    test_nested_error();

    return 0;
}
