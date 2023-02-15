namespace clanguml {
namespace t00056 {
template <typename T>
concept MaxFourBytes = sizeof(T) <= 4;

template <MaxFourBytes T> struct A {
    T a;
};
}
}