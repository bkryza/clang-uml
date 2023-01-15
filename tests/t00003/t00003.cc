#include <functional>
#include <string>
#include <thread>

namespace clanguml {
namespace t00003 {

class A {
public:
    A() = default;
    A(int i)
        : private_member{i}
    {
    }
    A(A &&) = default;
    A(const A &) = default;
    virtual ~A() = default;

    void basic_method() { }
    static int static_method() { return 0; }
    void const_method() const { }
    auto auto_method() { return 1; }

    auto double_int(const int i) { return 2 * i; }
    auto sum(const double a, const double b) { return a_ + b_ + c_; }

    auto default_int(int i = 12) { return i + 10; }
    std::string default_string(int i, std::string s = "abc")
    {
        return s + std::to_string(i);
    }

    static A create_from_int(int i) { return A(i); }

    int public_member;
    static int static_int;
    static const int static_const_int = 1;
    static const auto auto_member{10UL};

protected:
    void protected_method() { }

    int protected_member;

    std::function<bool(const int)> compare = [this](const int v) {
        return private_member > v;
    };

private:
    void private_method() { }

    int private_member;
    int a_, b_, c_;

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
};

int A::static_int = 1;

A::custom_thread1 A::start_thread1()
{
    return custom_thread1{[]() {}};
}

A::custom_thread2 A::start_thread2()
{
    return custom_thread2{[]() {}};
}

} // namespace t00003
} // namespace clanguml
