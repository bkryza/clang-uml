namespace clanguml {
namespace t20003 {

template <typename T> void m4(T p) { }

template <typename T> void m3(T p) { m4<T>(p); }

template <typename T> void m2(T p) { m3<T>(p); }

template <typename T> void m1(T p) { m2<T>(p); }
}
}
