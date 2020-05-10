#ifndef CIRCULAR_QUEUE_H_
#define CIRCULAR_QUEUE_H_

#include <array>

template <typename _T, size_t _Size>
class CircularQueue {
    std::array<_T, _Size> items;
    size_t head, tail;

public:
    explicit CircularQueue() : tail(0), head(0) {}

    bool empty() const {
        return tail == head;
    }

    void push(const _T & value) {
        items[tail] = value;
        tail = (tail + 1) % _Size;
    }

    _T pop() {
        const _T value = items[head];
        head = (head + 1) % _Size;
        return value;
    }

    void push_front(const _T & value) {
        head = (head - 1 + _Size) % _Size;
        items[head] = value;
    }

    _T first() const {
        return items[head];
    }

    _T last() const {
        return items[tail];
    }

    void clear() {
        tail = head;
    }
    
    size_t size() const {
        return (tail - head + _Size) % _Size;
    }
};

#endif /* CIRCULAR_QUEUE_H_ */
