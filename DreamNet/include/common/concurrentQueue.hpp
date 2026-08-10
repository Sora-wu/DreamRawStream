//
// Author: sora
// Mail: sora-wu@foxmail.com
//

#pragma once

#include <condition_variable>
#include <deque>
#include <mutex>
#include <optional>

template <typename T, typename Deque = std::deque<T>>
class ConcurrentQueue {
public:
    ConcurrentQueue() : maxSize_(std::numeric_limits<std::size_t>::max()) {}
    explicit  ConcurrentQueue(std::size_t maxSize) : maxSize_(maxSize ? maxSize : std::numeric_limits<std::size_t>::max()) {}

    [[nodiscard]] bool empty() const {
        std::lock_guard lock(mutex_);
        return queue_.empty();
    }

    [[nodiscard]] std::size_t size() const {
        std::lock_guard lock(mutex_);
        return queue_.size();
    }

    void close() {
        std::lock_guard lock(mutex_);
        closed_ = true;
        cvEmpty_.notify_all();
        cvFull_.notify_all();
    }

    void reopen() {
        std::lock_guard lock(mutex_);
        closed_ = false;
        cvEmpty_.notify_all();
        cvFull_.notify_all();
    }

    [[nodiscard]] bool isClosed() const {
        std::lock_guard lock(mutex_);
        return closed_;
    }

    bool push(T value) {
        std::unique_lock lock(mutex_);
        cvFull_.wait(lock, [&] { return queue_.size() < maxSize_ || closed_; });

        if (closed_) {
            return false;
        }

        queue_.push_back(std::move(value));
        cvEmpty_.notify_one();

        return true;
    }

    // 尝试推数据，不阻塞，如果满，返回 false
    bool try_push(T value) {
        std::lock_guard lock(mutex_);
        if (closed_ || queue_.size() >= maxSize_) {
            return false;
        }

        queue_.push_back(std::move(value));
        cvEmpty_.notify_one();
        return true;
    }

    // 尝试推数据，如果满，等待一段时间，超时返回 false
    bool try_push_for(T value, std::chrono::steady_clock::duration timeout) {
        std::unique_lock lock(mutex_);
        if (!cvFull_.wait_for(lock, timeout, [&] { return queue_.size() < maxSize_ || closed_; })) {
            return false;
        }
        if (closed_) {
            return false;
        }

        queue_.push_back(std::move(value));
        cvEmpty_.notify_one();
        return true;
    }

    // 尝试推数据，如果满，等待直到时间点，超时返回 false
    bool try_push_until(T value, std::chrono::steady_clock::time_point timeout) {
        std::unique_lock lock(mutex_);
        if (!cvFull_.wait_until(lock, timeout, [&] { return queue_.size() < maxSize_ || closed_; })) {
            return false;
        }
        if (closed_) {
            return false;
        }

        queue_.push_back(std::move(value));
        cvEmpty_.notify_one();
        return true;
    }

    std::optional<T> pop() {
        std::unique_lock lock(mutex_);
        cvEmpty_.wait(lock, [&] { return !queue_.empty() || closed_; });

        // 只有在队列为空且关闭时，才返回空。允许把关闭前遗留的数据消费完
        if (closed_ && queue_.empty()) {
            return std::nullopt;
        }

        T value = std::move(queue_.front());
        queue_.pop_front();
        cvFull_.notify_one();
        return value;
    }

    // 尝试取数据，不阻塞，如果没有，返回 nullopt
    std::optional<T> try_pop() {
        std::lock_guard lock(mutex_);
        if (queue_.empty()) {
            return std::nullopt;
        }
        T value = std::move(queue_.front());
        queue_.pop_front();
        cvFull_.notify_one();
        return value;
    }

    // 尝试取数据，如果没有，等待一段时间，超时返回 nullopt
    std::optional<T> try_pop_for(std::chrono::steady_clock::duration timeout) {
        std::unique_lock lock(mutex_);
        if (!cvEmpty_.wait_for(lock, timeout, [&] { return !queue_.empty() || closed_; })) {
            return std::nullopt;
        }
        // 只有在队列为空且关闭时，才返回空。允许把关闭前遗留的数据消费完
        if (closed_ && queue_.empty()) {
            return std::nullopt;
        }

        T value = std::move(queue_.front());
        queue_.pop_front();
        cvFull_.notify_one();
        return value;
    }

    // 尝试取数据，如果没有，等待直到时间点，超时返回 nullopt
    std::optional<T> try_pop_until(std::chrono::steady_clock::time_point timeout) {
        std::unique_lock lock(mutex_);
        if (!cvEmpty_.wait_until(lock, timeout, [&] { return !queue_.empty() || closed_; })) {
            return std::nullopt;
        }

        // 只有在队列为空且关闭时，才返回空。允许把关闭前遗留的数据消费完
        if (closed_ && queue_.empty()) {
            return std::nullopt;
        }
        
        T value = std::move(queue_.front());
        queue_.pop_front();
        cvFull_.notify_one();
        return value;
    }

    template<typename Container>
    void pop_all(Container& outC) {
        std::lock_guard lock(mutex_);
        if (queue_.empty()) {
            return;
        }

        if constexpr (std::is_same_v<std::decay_t<Container>, Deque>) {
            outC.swap(queue_);
            cvFull_.notify_all();
            return;
        }

        outC.insert(outC.end(), std::make_move_iterator(queue_.begin()), std::make_move_iterator(queue_.end()));
        queue_.clear();
        cvFull_.notify_all();
    }

    std::optional<T> peek() {
        std::unique_lock lock(mutex_);
        cvEmpty_.wait(lock, [&] { return !queue_.empty() || closed_; });
        if (queue_.empty() && closed_) {
            return std::nullopt;
        }

        return queue_.front();
    }

    std::optional<T> try_peek() {
        std::lock_guard lock(mutex_);
        if (queue_.empty()) {
            return std::nullopt;
        }

        return queue_.front();
    }

    template <typename F>
    bool visit_front(F&& visitor) const {
        std::lock_guard lock(mutex_);
        if (queue_.empty()) {
            return false;
        }

        visitor(queue_.front());
        return true;
    }

private:
    Deque queue_;
    mutable std::mutex mutex_;
    std::condition_variable cvEmpty_;
    std::condition_variable cvFull_;
    std::size_t maxSize_{};

    bool closed_ = false;
};