//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#pragma once

#include <algorithm>
#include <cstring>
#include <span>
#include <vector>

namespace Dream {
    class Buffer {
    public:
        Buffer() = default;

        // 可指定初始容量
        explicit Buffer(size_t initialCapacity) {
            buf_.resize(initialCapacity);
        }

        /**
         * 读操作：读取最多 size 个字节。
         * 返回一个 std::span<const char> 指向可读数据，同时内部会移动读指针。
         * 为保证返回的 span 始终有效，读取前会先将未读数据移动到缓冲区开头（compact）。
         * 若想避免频繁移动数据，可考虑使用 peek() + consume() 组合，
         * 并在需要时手动调用 compact() 回收已读空间。
         */
        [[nodiscard]] std::span<const char> read(size_t size) {
            // 调整缓冲区，使第一个字节是未读取过的（compact）
            if (readIndex_ > 0) {
                compact();
            }
            size_t avail = writeIndex_ - readIndex_; // readIndex_ 此时为 0
            size_t n = std::min(size, avail);
            std::span<const char> result(buf_.data(), n);
            readIndex_ = n; // 读指针前进
            return result;
        }

        /**
         * 写操作：将 data 中的数据追加到缓冲区。
         * 若可写空间不足，会先尝试回收已读空间（compact），若仍不足则自动扩容。
         */
        void write(std::span<const char> data) {
            if (data.empty()) return;
            ensureWritableSpace(data.size());
            std::memcpy(buf_.data() + writeIndex_, data.data(), data.size());
            writeIndex_ += data.size();
        }

        /**
         * 紧凑操作：将未读数据移动到缓冲区起始位置。
         * 调用后 readIndex_ == 0，writeIndex_ == 未读字节数。
         * 通常在需要释放更多可写空间时使用，也可主动调用以重置读取视图。
         */
        void compact() {
            if (readIndex_ > 0) {
                auto unreadSize = writeIndex_ - readIndex_;
                if (unreadSize > 0) {
                    std::memmove(buf_.data(), buf_.data() + readIndex_, unreadSize);
                }
                writeIndex_ = unreadSize;
                readIndex_ = 0;
            }
        }

        /// 可读字节数
        [[nodiscard]] size_t readableSize() const {
            return writeIndex_ - readIndex_;
        }

        /// 可写字节数（剩余容量）
        [[nodiscard]] size_t writableSize() const {
            return buf_.size() - writeIndex_;
        }

        /// 总容量
        [[nodiscard]] size_t capacity() const {
            return buf_.size();
        }

        /// 清空读写指针（不回收内存）
        void clear() {
            readIndex_ = 0;
            writeIndex_ = 0;
        }

        /**
         * 预览可读数据，不移动读指针。
         * 搭配 consume() 使用可以手动控制读取进度，且避免隐式 compact。
         */
        [[nodiscard]] std::span<const char> peek() const {
            return {buf_.data() + readIndex_, writeIndex_ - readIndex_};
        }

        /// 移动读指针，消耗 n 个字节（调用前应确保 n <= readableSize()）
        void consume(size_t n) {
            readIndex_ += std::min(n, readableSize());
        }

        /// 丢弃已读空间并尝试释放多余内存（使 capacity() == writeIndex_）
        void shrinkToFit() {
            compact();
            buf_.resize(writeIndex_);
            buf_.shrink_to_fit();
        }

    private:
        // 确保至少有 need 字节的可写空间
        void ensureWritableSpace(size_t need) {
            if (writableSize() >= need) {
                return;
            }

            // 先尝试通过 compact 回收已读区域的空间
            if (readIndex_ > 0) {
                compact();
                if (writableSize() >= need) {
                    return;
                }
            }

            // 扩容：至少翻倍，且不少于 writeIndex_ + need
            size_t newSize = std::max(writeIndex_ + need, buf_.size() * 2);
            if (newSize < writeIndex_ + need) {
                newSize = writeIndex_ + need; // 防止溢出
            }
            buf_.resize(newSize);
        }

        std::vector<char> buf_;
        size_t readIndex_ = 0;
        size_t writeIndex_ = 0;
    };
}