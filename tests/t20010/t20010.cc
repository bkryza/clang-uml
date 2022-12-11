#include <array>
#include <map>
#include <memory>
#include <vector>

namespace clanguml {
namespace t20010 {

struct A {
    void a1() { }
    void a2() { }
    void a3() { }
    void a4() { }
};

template <typename T> struct B {
    void b1() { a_.a1(); }
    void b2() { avector_.front().a2(); }
    void b3() { aptrvector_.front()->a3(); }
    void b4() { amap_.at(0).a4(); }

    A a_;
    std::vector<A> avector_;
    std::vector<std::unique_ptr<A>> aptrvector_;
    std::map<T, A> amap_;
};

void tmain()
{
    B<int> b;

    b.b1();
    b.b2();
    b.b3();
    b.b4();
}

}
}