#include "include/t20014.h"
#include "include/t20014_b.h"
#include "include/t20014_c.h"

namespace clanguml {
namespace t20014 {

void log(const char *msg) { }

int tmain()
{
    B b;
    C<B, int> c;

    b.b1(0, 1);
    b.b2(1, 2);

    c.c1(2, 3);

    return 0;
}
}
}