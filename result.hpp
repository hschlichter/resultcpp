#pragma once
#include <cstdlib>
#include <type_traits>

template<typename T, typename E>
struct Result;

template<typename T, typename E>
Result<T, E> ok(T val);

template<typename T, typename E>
Result<T, E> err(E err);

template<typename T, typename E>
struct Result {
    enum class Tag { 
        Ok, 
        Err 
    } tag;
    union {
        T value;
        E error;
    };

    template<typename F>
    auto map_err(F f) const -> Result<T, decltype(f(std::declval<E>()))> {
        static_assert(std::is_invocable_v<F, E>, "map_err: F must be callable with E");
        if (tag == Tag::Ok) {
            return ok<T, decltype(f(std::declval<E>()))>(value);
        } else {
            return err<T, decltype(f(std::declval<E>()))>(f(error));
        }
    }
};

template<typename T, typename E>
Result<T, E> ok(T val) {
    return Result<T, E> { 
        .tag = Result<T, E>::Tag::Ok,
        .value = val
    };
}

template<typename T, typename E>
Result<T, E> err(E err) {
    Result<T, E> r;
    r.tag = Result<T, E>::Tag::Err;
    r.error = err;

    return r;
}

template<typename E>
struct Result<void, E> {
    enum class Tag { Ok, Err } tag;
    E error;

    template<typename F>
    auto map_err(F f) const -> Result<void, decltype(f(std::declval<E>()))> {
        static_assert(std::is_invocable_v<F, E>, "map_err: F must be callable with E");
        if (tag == Tag::Err) {
            return err<void, decltype(f(std::declval<E>()))>(f(error));
        }

        return ok<void, decltype(f(std::declval<E>()))>();
    }
};

template<typename E>
Result<void, E> ok() {
    return Result<void, E> { 
        Result<void, E>::Tag::Ok, 
        {} 
    };
}

template<typename E>
Result<void, E> err(E err) {
    return Result<void, E> {
        Result<void, E>::Tag::Err,
        err
    };
}

template<typename Ptr, typename Error>
auto ok_or(Ptr ptr, Error e) -> Result<decltype(ptr), Error> {
    if (ptr != nullptr) {
        return ok<decltype(ptr), Error>(ptr);
    }

    return err<decltype(ptr), Error>(e);
}

template<typename E>
struct Display {
    static_assert(std::is_enum_v<E>, "Display<E>: E must be an enum type");
    static void print(E e);
};

// Fatal: print error and exit
template<typename T, typename E>
T unwrap(const Result<T, E>& res) {
    if (res.tag == Result<T, E>::Tag::Err) {
        Display<E>::print(res.error);
        std::exit(1);
    }

    return res.value;
}

// Specialization for void: fatal, no return value
template<typename E>
void unwrap(const Result<void, E>& res) {
    if (res.tag == Result<void, E>::Tag::Err) {
        Display<E>::print(res.error);
        std::exit(1);
    }
}

// Non-fatal: return fallback on error
template<typename T, typename E>
T unwrap_or(const Result<T, E>& res, T fallback) {
    if (res.tag == Result<T, E>::Tag::Err) {
        return fallback;
    }

    return res.value;
}

// Non-fatal with callback: invoke on_error on error
template<typename T, typename E, typename F>
T unwrap_or_else(const Result<T, E>& res, F on_error) {
    static_assert(std::is_invocable_v<F, E>, "unwrap_or_else: F must be callable with E");
    if (res.tag == Result<T, E>::Tag::Err) {
        return on_error(res.error);
    }

    return res.value;
}

// Non-fatal void specialization with callback: invoke on_error on error
template<typename E, typename F>
void unwrap_or_else(const Result<void, E>& res, F on_error) {
    static_assert(std::is_invocable_v<F, E>, "unwrap_or_else: F must be callable with E");
    if (res.tag == Result<void, E>::Tag::Err) {
        on_error(res.error);
    }
}

// .map(F f)` — transform the success value.  
// .and_then(F f)` — chain another fallible call.  
// .unwrap_or(default)` — return the value or a default.
