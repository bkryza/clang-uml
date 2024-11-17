#include "include/t20061.h"

namespace clanguml {
namespace t20061 {
class A {
public:
    int a() { return b.b(); }

    B b;
};

int tmain()
{
    A a;
    return a.a();
}
}
}