/**
 * @file BoundedQueue.hpp
 * @brief thread-safe bounded queue
 *
 * @author Seb
 * @date 2026-10-05
**/

#ifndef GAMEENGINE_SERVER_GAME_BOUNDEDQUEUE_HPP
#define GAMEENGINE_SERVER_GAME_BOUNDEDQUEUE_HPP

#include <cstddef>
#include <deque>
#include <mutex>
#include <utility>
#include <vector>

namespace server {

template <class T>
class BoundedQueue {
    public:
        explicit BoundedQueue(std::size_t capacity) : _capacity(capacity) {}

        bool push(T item)
        {
            std::scoped_lock lock(this->_mutex);
            if (this->_items.size() >= this->_capacity)
                return false;
            this->_items.push_back(std::move(item));
            return true;
        }

        std::vector<T> drain()
        {
            std::scoped_lock lock(this->_mutex);
            std::vector<T> result;
            result.reserve(this->_items.size());
            while (!this->_items.empty()) {
                result.push_back(std::move(this->_items.front()));
                this->_items.pop_front();
            }
            return result;
        }

    private:
        std::mutex _mutex;
        std::deque<T> _items;
        std::size_t _capacity;
};

} // namespace server

#endif // GAMEENGINE_SERVER_GAME_BOUNDEDQUEUE_HPP
