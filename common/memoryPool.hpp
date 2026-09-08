//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#pragma once

#include <cstdint>
#include <cstdlib>
#include <map>
#include <mutex>
#include <ranges>

class MemoryPool {
public:
    MemoryPool() = default;
    ~MemoryPool() {
        std::lock_guard lock(mutex_);

        for (auto& val : freeBlocks_ | std::views::values) {
            free(val);
        }

        freeBlocks_.clear();
    }

    MemoryPool(const MemoryPool&) = delete;
    MemoryPool& operator=(const MemoryPool&) = delete;

    char* allocate(uint32_t size) {
        if (size == 0) {
            return nullptr;
        }

        std::lock_guard lock(mutex_);
        auto it = freeBlocks_.lower_bound(size);
        if (it != freeBlocks_.end()) {
            char* ptr = it->second;
            freeBlocks_.erase(it);

            return ptr;
        }

        return static_cast<char*>(std::malloc(size));
    }

    void deallocate(char* ptr, uint32_t size) {
        if (!ptr || size == 0) {
            return;
        }

        std::lock_guard lock(mutex_);
        freeBlocks_.emplace(size, ptr);
    }

private:
    std::multimap<uint32_t, char*> freeBlocks_;
    std::mutex mutex_;
};

struct PooledBuffer {
    MemoryPool* pool = nullptr;
    char* data = nullptr;
    uint32_t size = 0;

    PooledBuffer() = default;
    PooledBuffer(MemoryPool* p, char* d, uint32_t s) : pool(p), data(d), size(s) {}
    PooledBuffer(PooledBuffer&& o) noexcept
        : pool(o.pool), data(o.data), size(o.size) {
        o.pool = nullptr;
        o.data = nullptr;
        o.size = 0;
    }

    PooledBuffer& operator=(PooledBuffer&& o) noexcept {
        if (this != &o) {
            release();
            pool = o.pool;
            data = o.data;
            size = o.size;
            o.pool = nullptr;
            o.data = nullptr;
            o.size = 0;
        }
        return *this;
    }

    ~PooledBuffer() { release(); }
    PooledBuffer(const PooledBuffer&) = delete;
    PooledBuffer& operator=(const PooledBuffer&) = delete;

private:
    void release() {
        if (pool && data) {
            pool->deallocate(data, size);
            pool = nullptr;
            data = nullptr;
            size = 0;
        }
    }
};