namespace clanguml {
namespace t00064 {

template <typename... Ts> struct type_list { };

template <typename T> struct head;
template <typename Head, typename... Tail>
struct head<type_list<Head, Tail...>> {
    using type = Head;
};

template <typename T> using head_t = typename head<T>::type;

template <typename, typename> class type_group_pair;
template <typename... First, typename... Second>
class type_group_pair<type_list<First...>, type_list<Second...>> { };

struct A { };
struct B { };
struct C { };

class R {
public:
    type_list<A, bool, int> aboolint;
    type_group_pair<type_list<float, double>, type_list<A, B, C>> abc;
};
}
}