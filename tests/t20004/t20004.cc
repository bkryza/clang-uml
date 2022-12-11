#include <string>

namespace clanguml {
namespace t20004 {

template <typename T> T m4(T p);

template <> [[maybe_unused]] int m4<int>(int p) { return p + 2; }

template <> [[maybe_unused]] unsigned long m4<unsigned long>(unsigned long p)
{
    return 1000 * p;
}

template <typename T> T m3(T p) { return m4<T>(p); }

template <typename T> T m2(T p) { return m3<T>(p); }

template <> [[maybe_unused]] std::string m2<std::string>(std::string p)
{
    return std::string{"m2"} + p;
}

template <typename T> T m1(T p) { return m2<T>(p); }

template <> [[maybe_unused]] float m1<float>(float p) { return 2 * p; }

template <> [[maybe_unused]] unsigned long m1<unsigned long>(unsigned long p)
{
    return m4<unsigned long>(p);
}

template <> [[maybe_unused]] std::string m1<std::string>(std::string p)
{
    return m2<std::string>(p);
}

int main()
{
    m1<float>(2.2);

    m1<unsigned long>(100);

    m1<std::string>("main");

    return m1<int>(0);
}
}
}
