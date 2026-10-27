#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

namespace mh4u::native_runtime {

// Sparse 32-bit guest address space. Only mapped 4 KiB pages consume physical memory.
// The page-pointer table is exposed to recomp.h's RC_PAGED fast-path.
class GuestMemory {
public:
    static constexpr std::uint32_t kPageSize = 4096;
    static constexpr std::uint32_t kPageCount = 1u << 20;

    GuestMemory();
    GuestMemory(const GuestMemory&) = delete;
    GuestMemory& operator=(const GuestMemory&) = delete;

    bool map(std::uint32_t address, std::uint32_t size);
    bool unmap(std::uint32_t address, std::uint32_t size);
    bool read(std::uint32_t address, std::uint32_t size, std::uint32_t& value) const noexcept;
    bool write(std::uint32_t address, std::uint32_t size, std::uint32_t value) noexcept;
    std::uint8_t** page_table() noexcept { return pages_.data(); }
    const std::uint8_t* page(std::uint32_t index) const noexcept;

private:
    bool valid_range(std::uint32_t address, std::uint32_t size) const noexcept;
    std::vector<std::uint8_t*> pages_;
    std::vector<std::unique_ptr<std::uint8_t[]>> owned_pages_;
};

} // namespace mh4u::native_runtime
