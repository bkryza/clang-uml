#include <string>
#include <vector>

namespace clanguml {
namespace t00009 {

template <typename T> class A {
public:
    T value;
};

class B {
public:
    A<int> aint;
    A<std::string> *astring;
    A<std::vector<std::string>> &avector;
};
} // namespace t00009
} // namespace clanguml
