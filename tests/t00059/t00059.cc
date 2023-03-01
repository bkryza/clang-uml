#include <string>

namespace clanguml {
namespace t00059 {

template <typename T>
concept fruit_c = requires(T t) {
                      T{};
                      t.get_name();
                  };

template <typename T>
concept apple_c = fruit_c<T> && requires(T t) { t.get_sweetness(); };

template <typename T>
concept orange_c = fruit_c<T> && requires(T t) { t.get_bitterness(); };

class gala_apple {
public:
    auto get_name() const -> std::string { return "gala"; }
    auto get_sweetness() const -> float { return 0.8; }
};

class empire_apple {
public:
    auto get_name() const -> std::string { return "empire"; }
    auto get_sweetness() const -> float { return 0.6; }
};

class lima_orange {
public:
    auto get_name() const -> std::string { return "lima"; }
    auto get_bitterness() const -> float { return 0.8; }
};

class valencia_orange {
public:
    auto get_name() const -> std::string { return "valencia"; }
    auto get_bitterness() const -> float { return 0.6; }
};

template <apple_c TA, orange_c TO> class fruit_factory {
public:
    auto create_apple() const -> TA { return TA{}; }
    auto create_orange() const -> TO { return TO{}; }
};

using fruit_factory_1 = fruit_factory<gala_apple, valencia_orange>;
using fruit_factory_2 = fruit_factory<empire_apple, lima_orange>;

struct R {
    fruit_factory_1 factory_1;
    fruit_factory_2 factory_2;
};
}
}