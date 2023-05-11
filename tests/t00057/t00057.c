#include "include/t00057.h"

struct t00057_A {
    int a1;
};

typedef struct t00057_B {
    int b1;
} t00057_B;

struct t00057_C {
    int c1;
};

union t00057_D {
    int d1;
    float d2;
};

struct t00057_E {
    int e;
    struct {
        int x;
        int y;
    } coordinates;
    union {
        int z;
        double t;
    } height;
};

typedef struct {
    int g1;
} t00057_G;

struct t00057_R {
    struct t00057_A a;
    t00057_B b;
    struct t00057_C *c;
    union t00057_D d;
    struct t00057_E *e;
    struct t00057_F *f;
    struct t00057_G *g;
};
