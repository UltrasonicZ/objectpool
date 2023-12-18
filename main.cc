#include "objectpool.h"
#include <iostream>
#include "singleobjectpool.h"
class Base {
public:
    virtual void copy() = 0;
};

class A {
public:
    A()
    : m_a(0)
    , m_b(0)
    , m_c("") {
     
    }
    A(int a, int b, std::string c)
    : m_a(a)
    , m_b(b)
    , m_c(c) {
    }
    void copy(int a, int b, std::string c) {
        m_a = a;
        m_b = b;
        m_c = c;
    }
    void print() {
        std::cout << "m_a :"  << m_a << std::endl;
        std::cout << "m_b :"  << m_b << std::endl;
        std::cout << "m_c :"  << m_c << std::endl;
    }
private:
    int m_a;
    int m_b;
    std::string m_c;
};
int main() {
    // ObjectPool<A> pool(3, 1, 1);
    // {
    //     {
    //         auto ptr = pool.get(100, 50, "gaozhou");
    //         ptr->Print();
    //     }
    //     auto ptr2 = pool.get(50, 25, "zhaochong");
    //     ptr2->Print();
    // }
    // std::cout << pool.size() << std::endl;

    // 使用单例模式获取 SingleObjectPool 实例
    SingleObjectPool<A> *singlepool = SingleObjectPool<A>::instance();

    // 初始化 SingleObjectPool
    singlepool->initialize(3, 1, 1);
    {
        // 获取对象
        auto obj1 = singlepool->get(100, 50, "gaozhou");
        obj1->print();
        std::cout << singlepool->size() << std::endl;
    }
    std::cout << singlepool->size() << std::endl;

    auto obj2 = singlepool->get(50, 25, "zhaochong");
    obj2->print();
    return 0;
}