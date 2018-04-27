#pragma once
#include <string>
#include <functional>
#include <thread>
#include <stdint.h>
#include <assert.h>

namespace beam {

// returns local timestamp in millisecond since the Epoch
uint64_t local_timestamp_msec();

// formatStr as for strftime (e.g. "%Y-%m-%d.%T"), if decimals==true, then .### milliseconds added
// returns bytes consumed
size_t format_timestamp(char* buffer, size_t bufferCap, const char* formatStr, uint64_t timestamp, bool formatMsec=true);

// formats current timestamp into std::string
inline std::string format_timestamp(const char* formatStr, uint64_t timestamp, bool formatMsec=true) {
    char buf[128];
    size_t n = format_timestamp(buf, 128, formatStr, timestamp, formatMsec);
    return std::string(buf, n);
}

// Converts bytes to base16 string, writes to dst buffer.
// dst must contain at least size*2 bytes + 1
char* to_hex(char* dst, const void* bytes, size_t size);

// Converts bytes to base16 string.
std::string to_hex(const void* bytes, size_t size);

/// Wraps member fn into std::function via lambda
template <typename R, typename ...Args, typename T> std::function<R(Args...)> bind_memfn(T* object, R(T::*fn)(Args...)) {
    return [object, fn](Args ...args) { return (object->*fn)(std::forward<Args>(args)...); };
}

/// Wrapper to bind member fn from inside this class methods
#define BIND_THIS_MEMFN(M) bind_memfn(this, &std::remove_pointer<decltype(this)>::type::M)

/// std::thread wrapper that spawns on demand
struct Thread {
    template <typename Func, typename ...Args> void start(Func func, Args ...args) {
        assert(!_thread);
        _thread = std::make_unique<std::thread>(func, std::forward<Args>(args)...);
    }

    void join() {
        if (_thread) {
            _thread->join();

            // object may be reused after join()
            _thread.reset();
        }
    }

    virtual ~Thread() {
        // yes, must be joined first
        assert(!_thread);
    }

private:
    std::unique_ptr<std::thread> _thread;
};

} //namespace

