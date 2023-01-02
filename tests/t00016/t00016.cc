namespace clanguml {
namespace t00016 {

template <typename> struct is_numeric {
    enum { value = false };
};

template <> struct is_numeric<float> {
    enum { value = true };
};

template <> struct is_numeric<char> {
    enum { value = true };
};

template <> struct is_numeric<unsigned int> {
    enum { value = true };
};

template <> struct is_numeric<int> {
    enum { value = true };
};

template <> struct is_numeric<bool> {
    enum { value = false };
};
} // namespace t00016
} // namespace clanguml
