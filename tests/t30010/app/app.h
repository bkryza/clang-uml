#pragma once

#include "../libraries/lib1/lib1.h"
#include "../libraries/lib2/lib2.h"
#include "../libraries/lib3/lib3.h"
#include "../libraries/lib4/lib4.h"

namespace clanguml {
namespace t30010 {

struct App {
    library1::A *a;
    library2::B<int> *b;
    library3::E e;

    void c(library4::C *) { }
};

}
}