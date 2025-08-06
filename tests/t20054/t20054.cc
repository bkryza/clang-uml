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

    struct {
        double c{1.0};
        double h{1.0};

        double celcius_to_fahrenheit(double cel) { return (9 * cel) / 5 + 32; }

    } units;

    void a() { aa.aa(); }

    AA aa;
};

void tmain()
{
    A a;

    a.a();
    a.units.celcius_to_fahrenheit(100);
}

} // namespace t20054
} // namespace clanguml