namespace clanguml {
namespace t00074 {
template <typename T>
concept fruit_c = requires(T t) {
                      T{};
                      t.get_name();
                  };

template <typename T>
concept apple_c = fruit_c<T> && requires(T t) { t.get_sweetness(); };

template <typename T>
concept orange_c = fruit_c<T> && requires(T t) { t.get_bitterness(); };

}
}