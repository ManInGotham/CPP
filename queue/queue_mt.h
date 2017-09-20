#pragma once

#include <queue>
#include <mutex>

using namespace std;

// A header-only implementation of a thread safe queue.
// Useful in producer-consumer related problems.
//
template <typename T>
class queue_mt {
private:
    queue<T> m_queue;
    mutex m_mutex;
    condition_variable m_conditionVar;

public:
    T Pop() {
        unique_lock<mutex> lock(m_mutex); // Will be unlocked at the end of the current scope
        
        // Wait for a signal until queue is not empty 
        while (m_queue.empty()) {
            m_conditionVar.wait(lock);
        }
         
        // Peek the front item an then pop (pop() returns void).
        auto item = m_queue.front();
        m_queue.pop();
        return item;
    }

    void Push(const T& item) {
        {
            unique_lock<mutex> lock(m_mutex);
            m_queue.push(item);
        }

        m_conditionVar.notify_one();
    }
};