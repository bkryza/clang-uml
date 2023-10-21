#include <future>

#include "include/t20038.h"

namespace clanguml {
namespace t20038 {

struct A {
    int a() { return 1; }

    int aa()
    {
        int i;
        // Repeat 10 times
        for (i = 0; i < 10;) {
            i += a();
        }
        return i;
    }

    int aaa() { return 3; }

    int aaaa() { return add(4, 4); }

    int aaaaa() { return 5; }
};

struct B {
    int b() { return a.a(); }

    int bb() { return a.aa(); }

    int bbb() { return a.aaa(); }

    int bbbb() { return a.aaaa(); }

    int bbbbb() { return a.aaaa(); }

    int wrap(int b) { return b; }

    A a;
};

int tmain()
{
    B b;

    // Nisl purus in mollis nunc sed id semper. Varius vel pharetra vel
    // turpis. Arcu cursus vitae congue mauris rhoncus. Risus feugiat in
    // ante metus dictum at tempor. Lacus vel facilisis volutpat est. Auctor
    // urna nunc id cursus metus aliquam. Diam sit amet nisl suscipit
    // adipiscing. Potenti nullam ac tortor vitae purus faucibus ornare
    // suspendisse sed. Lobortis feugiat vivamus at augue eget arcu dictum
    // varius. Non tellus orci ac auctor.
    if (true) {
        auto r = 0;
        // Repeat 5 times...
        while (r < 5) {
            r += b.b();
        }
        return r;
    }
    else {
        // ... or just once
        return 2 * b.b();
    }

    // \uml{skip}
    b.bb();

    // \uml{call clanguml::t20038::B::bbb()}
    auto bbb_future = std::async(std::launch::deferred, &B::bbb, b);

    bbb_future.wait();

    // This comment should be rendered only once
    b.wrap(b.bbbb());

    add_impl<double>(2, 2); // What is 2 + 2?

    // This is a generic comment about calling bbbbb()
    //
    // \uml{note:some_other_diagram[] This is specific for some_other_diagram}
    // \uml{note:t20038_sequence[] Calling B::bbbbb()}
    b.bbbbb();

    // This is a conditional operator
    return b.bbb() > 5 ? 0 : 1;
}
}
}