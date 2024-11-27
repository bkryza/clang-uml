namespace clanguml {
namespace t00004 {

enum Color { Red, Green, Blue };

class B {
public:
    enum AA { AA_1, AA_2, AA_3 };
    typedef enum { BB_1, BB_2, BB_3 } BB;
    typedef enum CC { CC_1, CC_2, CC_3 } CC_t;

    AA aa;
    BB bb;
    CC cc;
    Color *color;
};

class A {
public:
    void foo() const { }

    class AA {
    public:
        enum class Lights { Green, Yellow, Red };

        class AAA {
            Lights lights;
        };
    };

    void foo2() const { }
};

template <typename T> class C {
public:
    T t;

    class AA {
        class AAA { };

        enum class CCC { CCC_1, CCC_2 };
    };

    template <typename V> class B {
        V b;
    };

    B<int> b_int;

    enum class CC { CC_1, CC_2 };
};

namespace detail {
class D {
public:
    enum class AA { AA_1, AA_2, AA_3 };

    class DD { };
};
}

} // namespace t00004
} // namespace clanguml
