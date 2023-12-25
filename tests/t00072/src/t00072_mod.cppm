export module t00072.app;
export import :lib1;
export import :lib1.mod1;
export import :lib1.mod2;
export import :lib2;

export namespace clanguml::t00072 {
class A {
    int get() { return a; }

    int a;
};
}