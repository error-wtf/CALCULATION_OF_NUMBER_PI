#pragma once

#include <cstddef>

namespace picore {

// Placeholder far memory abstraction that does nothing.
class FarMemory {
public:
    explicit FarMemory(std::size_t /*size*/ = 0) {}
    void allocate(std::size_t /*size*/) {}
};

} // namespace picore
