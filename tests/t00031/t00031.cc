#include <memory>
#include <vector>

namespace clanguml {
namespace t00031 {

/// @uml{style[#back:lightgreen|yellow;header:blue/red]}
class A { };

/// @uml{style[#line.dotted:blue]}
enum B { one, two, three };

/// @uml{style[#pink;line:red;line.bold;text:red]}
template <typename T> class C {
    T ttt;
};

class D { };

struct R {
    /// @uml{style[#red,dashed,thickness=2]}
    A *aaa;

    /// @uml{composition}
    /// @uml{style[#green,dashed,thickness=4]}
    std::vector<B> bbb;

    void add_b(B b) { bbb.push_back(b); }

    /// @uml{style[#blue,dotted,thickness=8]}
    C<int> ccc;

    /// @uml{style[#blue,plain,thickness=16]}
    D *ddd;
};

} // namespace t00031
} // namespace clanguml
