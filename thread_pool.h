#pragma once

#include <functional>
#include <future>
#include <utility>

namespace picore {

// Minimal thread pool wrapper using std::async.
class ThreadPool {
public:
    ThreadPool(unsigned /*threads*/ = 1) {}

    template <class F, class... Args>
    auto enqueue(F&& f, Args&&... args)
        -> std::future<decltype(f(args...))> {
        // Use asynchronous launch policy to run on separate thread.
        return std::async(std::launch::async,
                          std::forward<F>(f),
                          std::forward<Args>(args)...);
    }
};

} // namespace picore
