#pragma once

#include "../libraries/common.h"

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