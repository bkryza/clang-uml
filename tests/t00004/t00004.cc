namespace clanguml {
namespace t00004 {

class A {
public:
    void foo() const {}

    class AA {
    public:
        enum class Lights { Green, Yellow, Red };

        class AAA {
        };
    };

    void foo2() const {}
};
}
}
