#include <thread>

namespace clanguml {
namespace t00051 {

template <typename F, typename FF = F> struct B : private std::thread {
    B(F &&f, FF &&ff)
        : f_{std::move(f)}
        , ff_{std::move(ff)}
    {
    }

    void f() { f_(); }
    void ff() { ff_(); }

    F f_;
    FF ff_;
};

class A {
public:
private:
    class custom_thread1 : private std::thread {
    public:
        template <class Function, class... Args>
        explicit custom_thread1(Function &&f, Args &&...args)
            : std::thread::thread(
                  std::forward<Function>(f), std::forward<Args>(args)...)
        {
        }
    };

    static custom_thread1 start_thread1();

    class custom_thread2 : private std::thread {
        using std::thread::thread;
    };

    static custom_thread2 start_thread2();

    auto start_thread3()
    {
        return B{[]() {}, []() {}};
    }

    auto get_function()
    {
        return []() {};
    }
};

A::custom_thread1 A::start_thread1()
{
    return custom_thread1{[]() {}};
}

A::custom_thread2 A::start_thread2()
{
    return custom_thread2{[]() {}};
}

}
}
