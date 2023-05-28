#include <array>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace clanguml {
namespace t30002 {
namespace A::AA {
namespace A1 {
struct CA { };
}
namespace A2 {
template <typename T> struct CB {
    T cb;
};
}
namespace A3 {
struct CC { };
}
namespace A4 {
struct CD { };
}
namespace A5 {
struct CE { };
}
namespace A6 {
struct CF { };
}
namespace A7 {
struct CG { };
}
namespace A8 {
struct CH { };
}
namespace A9 {
struct CI { };
}
namespace A10 {
struct CJ { };
}
namespace A11 {
struct CK { };
}
namespace A12 {
struct CL { };
}
namespace A13 {
struct CM { };
}
namespace A14 {
struct CN { };
}
namespace A15 {
struct CO { };
}
namespace A16 {
struct CP { };
}
namespace A17 {
struct CR { };
}
namespace A18 {
enum class S { s1, s2, s3 };
}
}
namespace B::BB::BBB {
class CBA : public A::AA::A6::CF {
public:
    A::AA::A1::CA *ca_;
    A::AA::A2::CB<int> cb_;
    std::shared_ptr<A::AA::A3::CC> cc_;
    std::map<std::string, std::unique_ptr<A::AA::A4::CD>> *cd_;
    std::array<A::AA::A15::CO, 5> co_;
    static A::AA::A16::CP *cp_;

    CBA() = default;

    CBA(A::AA::A14::CN *cn) { }

    friend A::AA::A17::CR;

    template <typename... Item> CBA(std::tuple<Item...> &items) { }

    void ce(const std::vector<A::AA::A5::CE> /*ce_*/) { }

    std::shared_ptr<A::AA::A7::CG> cg() { return {}; }

    template <typename T>
    void ch(std::map<T, std::shared_ptr<A::AA::A8::CH>> &ch_)
    {
    }

    template <typename T>
    std::map<T, std::shared_ptr<A::AA::A9::CI>> ci(T * /*t*/)
    {
        return {};
    }

    A::AA::A18::S s;
};

void cj(std::unique_ptr<A::AA::A10::CJ> /*cj_*/) { }

std::unique_ptr<A::AA::A11::CK> ck() { return {}; }

template <typename T>
void cl(std::map<T, std::shared_ptr<A::AA::A12::CL>> & /*ch_*/)
{
}

template <typename T> std::map<T, std::shared_ptr<A::AA::A13::CM>> cm()
{
    return {};
}
}
} // namespace t30002
} // namespace clanguml
