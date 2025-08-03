#include <memory>
#include <string>

namespace clanguml {
namespace t00097 {

class Component {
public:
    virtual ~Component() = default;
    virtual std::string operation() const = 0;
};

class ConcreteComponent : public Component {
public:
    std::string operation() const override { return "ConcreteComponent"; }
};

class Decorator : public Component {
protected:
    std::shared_ptr<Component> component_;

public:
    explicit Decorator(std::shared_ptr<Component> component)
        : component_(component)
    {
    }

    std::string operation() const override { return component_->operation(); }
};

class ConcreteDecoratorA : public Decorator {
public:
    explicit ConcreteDecoratorA(std::shared_ptr<Component> component)
        : Decorator(component)
    {
    }

    std::string operation() const override
    {
        return "ConcreteDecoratorA(" + Decorator::operation() + ")";
    }

    std::string added_behavior() const { return "Added behavior A"; }
};

class ConcreteDecoratorB : public Decorator {
public:
    explicit ConcreteDecoratorB(std::shared_ptr<Component> component)
        : Decorator(component)
    {
    }

    std::string operation() const override
    {
        return "ConcreteDecoratorB(" + Decorator::operation() + ")";
    }

    void additional_method() const { }
};

class Client {
    std::shared_ptr<Component> component_;

public:
    explicit Client(std::shared_ptr<Component> component)
        : component_(component)
    {
    }

    std::string execute() { return component_->operation(); }
};

} // namespace t00097
} // namespace clanguml