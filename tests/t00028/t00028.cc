#include <memory>
#include <vector>

namespace clanguml {
namespace t00028 {

/// \uml{note[top] A class note.}
class A { };

/// \uml{note[] B class note.}
class B { };

///
/// @uml{note:t00028_class[bottom] C class note.}
/// This is class C.
class C { };

/// \uml{note
/// D
/// class
/// note.}
class D { };

/// \uml{note E template class note.}
template <typename T> class E {
    T param;
};

/// \uml{note:other_diagram[left] G class note.}
class G { };

/// @uml{note[ bottom ] F enum note.}
enum class F { one, two, three };

/// \uml{note[right] R class note.}
class R {
    explicit R(C &c)
        : ccc(c)
    {
    }

    /// \uml{note[left] R contains an instance of A.}
    A aaa;

    /// \uml{note:other_diagram[right] R contains an pointer to B.}
    B *bbb;

    C &ccc;

    std::vector<std::shared_ptr<D>> ddd;

    E<int> eee;

    G **ggg;
};

} // namespace t00028
} // namespace clanguml
