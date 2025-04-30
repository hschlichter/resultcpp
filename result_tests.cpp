#include "catch2/catch_test_macros.hpp"
#include <type_traits>
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

TEST_CASE("ok and err basic behavior", "Result") {
    SECTION("ok holds value") {
        auto r = ok<int, TestError>(123);
        REQUIRE(r.tag == Result<int, TestError>::Tag::Ok);
        REQUIRE(r.value == 123);
    }
    SECTION("err holds error") {
        auto r = err<int, TestError>(TestError::B);
        REQUIRE(r.tag == Result<int, TestError>::Tag::Err);
        REQUIRE(r.error == TestError::B);
    }
}

TEST_CASE("map_err transforms error type", "Result") {
    auto r_ok = ok<int, TestError>(5).map_err([] (TestError) { return RootError::D; });
    REQUIRE(r_ok.tag == Result<int, RootError>::Tag::Ok);
    REQUIRE(r_ok.value == 5);

    auto r_err = err<int, TestError>(TestError::A).map_err([](TestError){ return RootError::C; });
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
    auto r_ok = ok<int, TestError>(7);
    REQUIRE(unwrap_or(r_ok, -1) == 7);

    auto r_err = err<int, TestError>(TestError::A);
    REQUIRE(unwrap_or(r_err, -1) == -1);
}

TEST_CASE("unwrap_or_else invokes callback on error", "unwrap_or_else") {
    auto r_ok = ok<int, TestError>(42);
    REQUIRE(unwrap_or_else(r_ok, [&](TestError){ return -2; }) == 42);

    auto r_err = err<int, TestError>(TestError::B);
    REQUIRE(unwrap_or_else(r_err, [&](TestError e){ return static_cast<int>(e) * 2; }) == static_cast<int>(TestError::B) * 2);
}

TEST_CASE("void Result specialization and helpers", "Result<void>") {
    // Ok<void>
    Result<void, TestError> v_ok = ok<TestError>();
    REQUIRE(v_ok.tag == Result<void, TestError>::Tag::Ok);

    // Err<void>
    Result<void, TestError> v_err = err<void, TestError>(TestError::A);
    REQUIRE(v_err.tag == Result<void, TestError>::Tag::Err);

    // unwrap_or for void (should not crash)
    // unwrap_or(v_err, (void*)nullptr);

    // unwrap_or_else for void
    bool called = false;
    unwrap_or_else(v_err, [&](TestError){ called = true; });
    REQUIRE(called);
}

TEST_CASE(".unwrap from result") {
    auto r = ok<int, TestError>(42);
    REQUIRE(r.unwrap() == 42);
}

TEST_CASE(".unwrap result errors") {
    auto r = err<int, TestError>(TestError::A);
    REQUIRE(r.tag == Result<int, TestError>::Tag::Err);
    // r.unwrap();
}

TEST_CASE("map value type int into char") {
    auto r = ok<int, TestError>(42)
        .map([] (int i) {
            return static_cast<char>(i);
        })
        .unwrap();
    REQUIRE(r == '*');
}

TEST_CASE("verify types of map before and after") {
    auto r0 = ok<int, TestError>(42);
    using R0 = decltype(r0);
    STATIC_REQUIRE(std::is_same<typename R0::value_type, int>());

    auto r1 = r0.map([] (int i) {
        return static_cast<char>(i);
    });
    using R1 = decltype(r1);
    STATIC_REQUIRE(std::is_same<typename R1::value_type, char>());
}

TEST_CASE() {
    auto h = ok<int, TestError>(22)
        .and_then([] (int i) {
            REQUIRE(i == 22);
            return ok<int, TestError>(i * 2);
        })
        .unwrap();
    REQUIRE(h == 44);
}

TEST_CASE("match operator for result") {
    auto val_ok = false;
    auto val_err = false;
    auto r = ok<int, TestError>(42);
    REQUIRE(r.tag == Result<int, TestError>::Tag::Ok);
    match(
        r, 
        [&] (int i) {
            REQUIRE(i == 42);
            val_ok = true;
        },
        [&] (TestError) {
            val_err = true;
        }
    );
    REQUIRE(val_ok == true);
    REQUIRE(val_err == false);

    val_ok = false;
    val_err = false;
    r = err<int, TestError>(TestError::A);
    REQUIRE(r.tag == Result<int, TestError>::Tag::Err);
    match(
        r,
        [&] (int) {
            val_ok = true;
        }, 
        [&] (TestError e) {
            REQUIRE(e == TestError::A);
            val_err = true;
        }
    );
    REQUIRE(val_ok == false);
    REQUIRE(val_err == true);
}
