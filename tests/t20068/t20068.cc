namespace clanguml::t20068 {

int baz() { return 0; }
int bar() { return baz(); }
int foo() { return bar(); }

int a() { return 1; }
int aa() { return a(); }
int ab() { return a(); }
int aaa() { return aa() + ab(); }

int c() { return 0; }
int cc() { return c(); }
int tmain() { return cc(); }

} // namespace clanguml::t20068
