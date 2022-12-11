#include <string>
#include <utility>

namespace clanguml {
namespace t20007 {

template <typename First, typename... Args> struct Adder {
    First add(First &&arg, Args &&...args) { return (arg + ... + args); }
};

void tmain()
{
    using namespace std::string_literals;

    Adder<int, int> adder1;
    Adder<int, float, double> adder2;
    Adder<std::string, std::string, std::string> adder3;

    [[maybe_unused]] auto res1 = adder1.add(2, 2);
    [[maybe_unused]] auto res2 = adder2.add(1, 2.0, 3.0);
    [[maybe_unused]] auto res3 = adder3.add("one"s, "two"s, "three"s);
}

}
}