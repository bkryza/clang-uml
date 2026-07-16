#pragma once

struct t30011_A {
    int a;
};

#define DECLARE_T30011_STRUCT(name)                                            \
    struct t30011_##name {                                                     \
        int member_##name;                                                     \
    }
