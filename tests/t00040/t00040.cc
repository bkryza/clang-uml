namespace clanguml::t00040 {

struct B { };

struct A {
public:
    int get_a() { return hidden_a_; }

protected:
    int ii_;

private:
    void foo() { }

    int hidden_a_;
};

class AA : public A {
public:
};

class AAA : public AA {
public:
    int get_aaa() { return hidden_aaa_; }
    B *b;

private:
    int hidden_aaa_;
};

struct R {
    void foo(A *a) { }
};

} // namespace clanguml::t00040
