#include <concepts>
#include <memory>
#include <string>

namespace clanguml {
namespace t00098 {

template <typename T>
concept Component = requires(const T &t) {
    { t.operation() } -> std::convertible_to<std::string>;
};

struct ConcreteComponent {
    std::string operation() const { return "ConcreteComponent"; }
};

template <Component Inner> class ConcreteDecoratorA {
    Inner inner_;

public:
    explicit ConcreteDecoratorA(Inner inner)
        : inner_(std::move(inner))
    {
    }

    std::string operation() const
    {
        return "ConcreteDecoratorA(" + inner_.operation() + ")";
    }

    std::string added_behavior() const { return "Added behavior A"; }
};

template <Component Inner> class ConcreteDecoratorB {
    Inner inner_;

public:
    explicit ConcreteDecoratorB(Inner inner)
        : inner_(std::move(inner))
    {
    }

    std::string operation() const
    {
        return "ConcreteDecoratorB(" + inner_.operation() + ")";
    }

    void additional_method() const { }
};

template <Component C> class Client {
    C component_;

public:
    explicit Client(C component)
        : component_(std::move(component))
    {
    }

    std::string execute() { return component_.operation(); }
};

using BAClient =
    Client<ConcreteDecoratorB<ConcreteDecoratorA<ConcreteComponent>>>;

struct R {
    BAClient client;
};

int main()
{
    R r{.client = Client{
            ConcreteDecoratorB{ConcreteDecoratorA{ConcreteComponent{}}}}};

    std::puts(r.client.execute().c_str());

    return 0;
}
} // namespace t00098
} // namespace clanguml