namespace clanguml {
namespace t00078 {

struct Base { };

struct D { };
struct E { };
struct A : public Base {
    D d;
    E *e;
};

struct B {
    A *a;
};

struct C {
    A a;
};

}
}