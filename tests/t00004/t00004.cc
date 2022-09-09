namespace clanguml {
namespace t00004 {

class B {
public:
    enum AA { AA_1, AA_2, AA_3 };
};

class A {
public:
    void foo() const { }

    class AA {
    public:
        enum class Lights { Green, Yellow, Red };

        class AAA {
        };
    };

    void foo2() const { }
};

template <typename T> class C {
public:
    T t;

    class AA {
        class AAA {
        };

        enum class CCC { CCC_1, CCC_2 };
    };

    enum class CC { CC_1, CC_2 };
};

}
}
