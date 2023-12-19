#include <iostream>
#include <queue>
#include <mutex>
#include <functional>

template <typename T>
class SingleObjectPool {
public:
    static SingleObjectPool* instance(size_t poolsize, size_t extendsize, size_t destroysize) {
        static SingleObjectPool instance(poolsize, extendsize, destroysize);
        return &instance;
    }

    template <typename... Args>
    std::shared_ptr<T> get(Args&&... args) {
        std::shared_ptr<T> pobject;
        {
            std::lock_guard<std::mutex> lock(m_poolmutex);
            if (m_objectpool.size() > m_poolsize) {
                destroy(m_destroysize);
            }
            if (m_objectpool.empty()) {
                extend(m_extendsize);
            }
            pobject = m_objectpool.front();
            m_objectpool.pop();
        }
        if(pobject)
            pobject->copy(std::forward<Args>(args)...);
        return pobject;
    }

    bool empty() const {
        return m_objectpool.empty();
    }

    size_t size() const {
        return m_objectpool.size();
    }

private:
    SingleObjectPool(size_t poolsize, size_t extendsize, size_t destroysize) {
        m_poolsize = poolsize;
        m_extendsize = extendsize;
        m_destroysize = destroysize;

        m_objectdeleter = [&](T* pobject) {
            if (m_isdelete) {
                delete pobject;
            }
            else if(m_isdestory) {
                delete pobject;
                m_isdestory = false;
            }
            else
            {
                m_objectpool.push(std::shared_ptr<T>(pobject, m_objectdeleter));
            }
        };
        for (int i = 0; i < m_poolsize; ++i) {
            std::shared_ptr<T> object(new T(), m_objectdeleter);
            m_objectpool.push(object);
        }
    }
    
    ~SingleObjectPool() {
        m_isdelete = true;
    }

    SingleObjectPool(const SingleObjectPool&) = delete;
    SingleObjectPool& operator=(const SingleObjectPool&) = delete;

    void extend(const size_t num) {
        for (size_t i = 0; i < num; ++i) {
            m_objectpool.push(std::shared_ptr<T>(new T(), m_objectdeleter));
        }
    }

    void destroy(const size_t num) {
        for (size_t i = 0; i < num; ++i) {
            if (m_objectpool.empty())
                return;
            std::shared_ptr<T> pobject = std::move(m_objectpool.front());
            m_objectpool.pop();
            m_isdestory = true;
            pobject.reset();
        }
    }

    volatile bool m_isdestory = false;
    volatile bool m_isdelete = false;
    std::queue<std::shared_ptr<T>> m_objectpool;
    size_t m_poolsize;
    size_t m_extendsize;
    size_t m_destroysize;
    std::mutex m_poolmutex;
    std::function<void(T* pObject)> m_objectdeleter;
};