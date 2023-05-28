#pragma once

#include "../libraries/lib1/lib1.h"
#include "../libraries/lib2/lib2.h"
#include "../libraries/lib3/lib3.h"
#include "../libraries/lib4/lib4.h"

struct t30011_App {
    struct t30011_A a;
    struct t30011_B *b;
    enum t30011_E e;
};

void c(struct t30011_App *app, struct t30011_C *c) { }
