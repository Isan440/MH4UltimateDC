#include "guest_memory.h"

#include <algorithm>
#include <cstring>
#include <limits>
#include <new>

namespace mh4u::native_runtime {

GuestMemory::GuestMemory() : pages_(kPageCount, nullptr) {}

bool GuestMemory::valid_range(std::uint32_t address, std::uint32_t size) const noexcept {
    if (size == 0) return false;
    const std::uint64_t end = static_cast<std::uint64_t>(address) + size;
    return end <= (std::uint64_t{1} << 32) && end > address;
}

bool GuestMemory::map(std::uint32_t address, std::uint32_t size) {
    if (!valid_range(address, size)) return false;
    const std::uint32_t first = address / kPageSize;
    const std::uint32_t last = static_cast<std::uint32_t>(
        (static_cast<std::uint64_t>(address) + size - 1) / kPageSize);

    for (std::uint32_t i = first; i <= last; ++i) {
        if (pages_[i] != nullptr) return false;
    }

    std::vector<std::unique_ptr<std::uint8_t[]>> pending;
    const auto count = static_cast<std::size_t>(last - first) + 1;
    try {
        pending.reserve(count);
        owned_pages_.reserve(owned_pages_.size() + count);
        for (std::uint32_t i = first; i <= last; ++i) {
            auto p = std::make_unique<std::uint8_t[]>(kPageSize);
            std::memset(p.get(), 0, kPageSize);
            pending.push_back(std::move(p));
        }
    } catch (const std::bad_alloc&) {
        return false;
    }

    for (std::uint32_t i = first; i <= last; ++i) {
        pages_[i] = pending[i - first].get();
        owned_pages_.push_back(std::move(pending[i - first]));
    }
    return true;
}

bool GuestMemory::unmap(std::uint32_t address, std::uint32_t size) {
    if (!valid_range(address, size) || address % kPageSize != 0 ||
        (size % kPageSize != 0 && static_cast<std::uint64_t>(address) + size != (std::uint64_t{1} << 32))) {
        return false;
    }
    const std::uint32_t first = address / kPageSize;
    const std::uint32_t count = size / kPageSize + ((size % kPageSize) ? 1 : 0);
    if (static_cast<std::uint64_t>(first) + count > kPageCount) return false;
    for (std::uint32_t i = first; i < first + count; ++i) {
        if (pages_[i] == nullptr) return false;
    }
    for (std::uint32_t i = first; i < first + count; ++i) {
        auto* raw = pages_[i];
        pages_[i] = nullptr;
        const auto it = std::find_if(owned_pages_.begin(), owned_pages_.end(),
            [raw](const auto& p) { return p.get() == raw; });
        if (it != owned_pages_.end()) owned_pages_.erase(it);
    }
    return true;
}

bool GuestMemory::read(std::uint32_t address, std::uint32_t size,
                       std::uint32_t& value) const noexcept {
    if (size != 1 && size != 2 && size != 4) return false;
    const std::uint64_t end = static_cast<std::uint64_t>(address) + size;
    if (end > (std::uint64_t{1} << 32)) return false;
    value = 0;
    for (std::uint32_t i = 0; i < size; ++i) {
        const std::uint64_t a = static_cast<std::uint64_t>(address) + i;
        const auto page_index = static_cast<std::uint32_t>(a / kPageSize);
        if (pages_[page_index] == nullptr) return false;
        value |= static_cast<std::uint32_t>(pages_[page_index][a % kPageSize]) << (8 * i);
    }
    return true;
}

bool GuestMemory::write(std::uint32_t address, std::uint32_t size, std::uint32_t value) noexcept {
    if (size != 1 && size != 2 && size != 4) return false;
    const std::uint64_t end = static_cast<std::uint64_t>(address) + size;
    if (end > (std::uint64_t{1} << 32)) return false;
    for (std::uint32_t i = 0; i < size; ++i) {
        const std::uint64_t a = static_cast<std::uint64_t>(address) + i;
        if (pages_[a / kPageSize] == nullptr) return false;
    }
    for (std::uint32_t i = 0; i < size; ++i) {
        const std::uint64_t a = static_cast<std::uint64_t>(address) + i;
        pages_[a / kPageSize][a % kPageSize] = static_cast<std::uint8_t>(value >> (8 * i));
    }
    return true;
}

const std::uint8_t* GuestMemory::page(std::uint32_t index) const noexcept {
    return index < kPageCount ? pages_[index] : nullptr;
}

} // namespace mh4u::native_runtime
