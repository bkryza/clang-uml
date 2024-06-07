namespace clanguml {
namespace t20054 {

struct A {
    struct AA {
        struct AAA {
            int aaa() { return 3; }
        };

        struct BBB {
            int bbb() { return 4; }
        };

        int aa() { return aaa.aaa(); }

        int bb() { return bbb.bbb(); }

        AAA aaa;
        BBB bbb;
    };

    void a() { aa.aa(); }

    AA aa;
};

void tmain()
{
    A a;

    a.a();
}

} // namespace t20054
} // namespace clanguml