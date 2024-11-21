#include <cstdarg>

namespace clanguml::t20062 {
namespace ns1 {
namespace ns2 {
int func2();
} // namespace ns2

int func1() { return ns2::func2(); }

namespace ns2 {

int add(int count...)
{
    int result = 0;
    std::va_list args;
    va_start(args, count);
    for (int i = 0; i < count; ++i)
        result += va_arg(args, int);
    va_end(args);
    return result;
}

int multiply_impl(int count, va_list args)
{
    int result = 1;
    for (int i = 0; i < count; ++i)
        result *= va_arg(args, int);

    return result;
}

int multiply(int count, ...)
{
    std::va_list args;
    va_start(args, count);
    int result = multiply_impl(count, args);
    va_end(args);
    return result;
}

int func2() { return add(1, 2, 3, 4, 5) + multiply(6, 7, 8); }
} // namespace ns2
} // namespace ns1

int tmain() { return ns1::func1(); }

} // namespace clanguml::t20062