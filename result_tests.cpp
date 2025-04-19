#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>
#include "result.hpp"  // include the implementation file directly for testing

// Define a test error enum to use with Result
enum class TestError {
    A, 
    B 
};

enum class RootError {
    C,
    D,
};

// Provide a Display specialization for TestError to satisfy print calls
template<>
struct Display<TestError> {
    static void print(TestError) {
        // no-op for tests
    }
};

template<>
struct Display<RootError> {
    static void print(RootError) {
        // no-op for tests
    }
};

TEST_CASE("Ok and Err basic behavior", "Result") {
    SECTION("Ok holds value") {
        auto r = Ok<int, TestError>(123);
        REQUIRE(r.tag == Result<int, TestError>::Tag::Ok);
        REQUIRE(r.value == 123);
    }
    SECTION("Err holds error") {
        auto r = Err<int, TestError>(TestError::B);
        REQUIRE(r.tag == Result<int, TestError>::Tag::Err);
        REQUIRE(r.error == TestError::B);
    }
}

TEST_CASE("map_err transforms error type", "Result") {
    auto r_ok = Ok<int, TestError>(5).map_err([] (TestError) { return RootError::D; });
    REQUIRE(r_ok.tag == Result<int, RootError>::Tag::Ok);
    REQUIRE(r_ok.value == 5);

    auto r_err = Err<int, TestError>(TestError::A).map_err([](TestError){ return RootError::C; });
    REQUIRE(r_err.tag == Result<int, RootError>::Tag::Err);
    REQUIRE(r_err.error == RootError::C);
}

TEST_CASE("ok_or returns pointer or error fallback", "ok_or") {
    int x = 10;
    auto p_ok = ok_or(&x, TestError::A);
    REQUIRE(p_ok.tag == Result<int*, TestError>::Tag::Ok);
    REQUIRE(*p_ok.value == 10);

    auto p_err = ok_or(nullptr, TestError::B);
    REQUIRE(p_err.tag == Result<nullptr_t, TestError>::Tag::Err);
    REQUIRE(p_err.error == TestError::B);
}

TEST_CASE("unwrap_or returns value or fallback", "unwrap_or") {
    auto r_ok = Ok<int, TestError>(7);
    REQUIRE(unwrap_or(r_ok, -1) == 7);

    auto r_err = Err<int, TestError>(TestError::A);
    REQUIRE(unwrap_or(r_err, -1) == -1);
}

TEST_CASE("unwrap_or_else invokes callback on error", "unwrap_or_else") {
    auto r_ok = Ok<int, TestError>(42);
    REQUIRE(unwrap_or_else(r_ok, [&](TestError){ return -2; }) == 42);

    auto r_err = Err<int, TestError>(TestError::B);
    REQUIRE(unwrap_or_else(r_err, [&](TestError e){ return static_cast<int>(e) * 2; }) == static_cast<int>(TestError::B) * 2);
}

TEST_CASE("void Result specialization and helpers", "Result<void>") {
    // Ok<void>
    Result<void, TestError> v_ok = Ok<TestError>();
    REQUIRE(v_ok.tag == Result<void, TestError>::Tag::Ok);

    // Err<void>
    Result<void, TestError> v_err = Err<void, TestError>(TestError::A);
    REQUIRE(v_err.tag == Result<void, TestError>::Tag::Err);

    // unwrap_or for void (should not crash)
    unwrap_or(v_err, (void*)nullptr);

    // unwrap_or_else for void
    bool called = false;
    unwrap_or_else(v_err, [&](TestError){ called = true; });
    REQUIRE(called);
}

