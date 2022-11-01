namespace clanguml {
namespace t20004 {

template <typename T> T m4(T p);

template <typename T> T m3(T p) { return m4<T>(p); }

template <typename T> T m2(T p) { return m3<T>(p); }

template <typename T> T m1(T p) { return m2<T>(p); }

template <> [[maybe_unused]] int m4<int>(int p) { return p + 2; }

int main() { return m1<int>(0); }
}
}
