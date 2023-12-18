#include <mutex>
#include <functional>
#include <memory>
#include <queue>
#include <iostream>

template<typename T>
class ObjectPool
{
public:    
    explicit ObjectPool(size_t poolsize, size_t extendsize, size_t destroysize) 
    : m_poolsize(poolsize)
    , m_extendsize(extendsize)
    , m_destroysize(destroysize)
    {
        m_objectdeleter = [&](T* pobject) {
            if (m_isdestory)
                delete pobject;
            else
            {
                std::lock_guard<std::mutex> lock(m_poolmutex);
                m_objectpool.push(std::shared_ptr<T>(pobject, m_objectdeleter));
                std::cout << "after delete size : " << m_objectpool.size() << std::endl;
            }
        };
        std::cout << "m_poolsize size : " << m_poolsize << std::endl;
        std::cout << "m_extendsize size : " << m_extendsize << std::endl;
        std::cout << "m_destroysize size : " << m_destroysize << std::endl;
        
        for(int i = 0; i < m_poolsize; ++i) {
            std::shared_ptr<T> object(new T(), m_objectdeleter);
            m_objectpool.push(object);
        }
        std::cout << "m_objectpool.size() : " << m_objectpool.size() << std::endl;
    }
    ~ObjectPool()
    {
        m_isdestory = true;
    }  
    template<typename... Args>
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
            std::cout << "m_objectpool.size() : " << m_objectpool.size() << std::endl;
            pobject = m_objectpool.front();
            m_objectpool.pop();
            std::cout << "m_objectpool.size() : " << m_objectpool.size() << std::endl;
        }
        // pobject.reset(new T(std::forward<Args>(args)...), m_objectdeleter);
        pobject->copy(std::forward<Args>(args)...);
        return pobject;
    }
    void extend(const size_t num) {
        std::shared_ptr<T> object(new T(), m_objectdeleter);
        for (size_t i = 0; i < num; ++i) {
            m_objectpool.push(object);
        }
    }
    void destroy(const size_t num) {
        std::shared_ptr<T> object(new T(), m_objectdeleter);
        for (size_t i = 0; i < num; ++i) {
            if(m_objectpool.empty())
                return;
            m_objectpool.pop();
        }
    }
    bool empty() 
    {
        return m_objectpool.empty(); 
    } 
    size_t size() 
    {
        return m_objectpool.size(); 
    }
private:
    volatile bool m_isdestory = false;
    std::queue<std::shared_ptr<T>> m_objectpool;
    size_t m_poolsize;
    size_t m_extendsize;
    size_t m_destroysize;
    std::mutex m_poolmutex;    
    std::function<void(T* pObject)> m_objectdeleter;
};