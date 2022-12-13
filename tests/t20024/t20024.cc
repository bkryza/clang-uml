namespace clanguml {
namespace t20024 {

enum enum_a { zero = 0, one = 1, two = 2, three = 3 };

enum class colors { red, orange, green };

struct A {
    int select(enum_a v)
    {
        switch (v) {
        case zero:
            return a0();
        case one:
            return a1();
        case two:
            return a2();
        default:
            return a3();
        }
    }

    int a0() { return 0; }
    int a1() { return 1; }
    int a2() { return 2; }
    int a3() { return 3; }
};

struct B {
    void select(colors c)
    {
        switch (c) {
        case colors::red:
            red();
            break;
        case colors::orange:
            orange();
            break;
        case colors::green:
            green();
            break;
        default:
            grey();
        }
    }

    void red() { }
    void orange() { }
    void green() { }
    void grey() { }
};

int tmain()
{
    A a;
    B b;

    a.select(enum_a::two);

    b.select(colors::green);

    return 0;
}
}
}