/**
 * Copyright (c) 2018, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 * Boost Software License - Version 1.0
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */
#ifndef CONCURRENT_QUEUE_BITS_HPP
#define CONCURRENT_QUEUE_BITS_HPP

#include "../concurrent_queue.hpp"

namespace hadoken {


template <typename T, typename ThreadModel, typename Allocator>
inline concurrent_queue_stl_mut<T, ThreadModel, Allocator>::concurrent_queue_stl_mut(const Allocator& allocator)
    : _qmut(), _qcond(), _dek(allocator), _buffer_capacity(0), _small_allocation(128) {}

template <typename T, typename ThreadModel, typename Allocator>
inline void concurrent_queue_stl_mut<T, ThreadModel, Allocator>::push(T element) {
    {
        std::lock_guard<std::mutex> l(_qmut);

        _dek.push_back(std::move(element));
        _buffer_capacity += 1;
        _qcond.notify_one();
    }
}


template <typename T, typename ThreadModel, typename Allocator>
template <typename Duration>
inline optional<T> concurrent_queue_stl_mut<T, ThreadModel, Allocator>::try_pop(const Duration& d) {
    optional<T> res;

    {
        bool timeout = false;
        std::unique_lock<decltype(_qmut)> l(_qmut);

        while (!timeout) {
            if (!_dek.empty()) {
                // dequeue if work is available
                res = std::move(_dek.front());
                _dek.pop_front();
                return res;
            } else {
                // deque and STL contains do not release memory
                // even empty and shrink_to_fit() is not always correctly implemented
                // if the buffer is larger than pure arbitrary value
                if (_buffer_capacity > _small_allocation) {
                    decltype(_dek) tmp;
                    _dek.swap(tmp);
                    _buffer_capacity = 0;
                }
            }


            if (_qcond.wait_for(l, d) == std::cv_status::timeout) {
                timeout = true;
            }
        }
    }
    return res;
}

template <typename T, typename ThreadModel, typename Allocator>
inline optional<T> concurrent_queue_stl_mut<T, ThreadModel, Allocator>::try_pop() {
    return try_pop(std::chrono::milliseconds(0));
}


template <typename T, typename ThreadModel, typename Allocator>
bool concurrent_queue_stl_mut<T, ThreadModel, Allocator>::empty() const {
    std::lock_guard<decltype(_qmut)> l(_qmut);
    return _dek.empty();
}

template <typename T, typename ThreadModel, typename Allocator>
std::size_t concurrent_queue_stl_mut<T, ThreadModel, Allocator>::size() const {
    std::lock_guard<decltype(_qmut)> l(_qmut);
    return _dek.size();
}


template <typename T, typename ThreadModel, typename Allocator>
void concurrent_queue_stl_mut<T, ThreadModel, Allocator>::set_small_buffer_size(std::uint64_t v) {
    std::lock_guard<decltype(_qmut)> l(_qmut);
    _small_allocation = v;
}


} // namespace hadoken

#endif // CONCURRENT_QUEUE_HPP
