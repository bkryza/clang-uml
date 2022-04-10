#include "../include/lib1/lib1.h"
#include "../include/lib2/lib2.h"

namespace clanguml::t40002 {

int foo() { return lib1::foo() + lib2::foo(); }

}