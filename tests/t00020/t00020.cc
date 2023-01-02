#include <memory>

namespace clanguml {
namespace t00020 {

class ProductA {
public:
    virtual ~ProductA() = default;
    virtual bool sell(int price) const = 0;
};

class ProductA1 : public ProductA {
public:
    bool sell(int price) const override { return price > 1000; }
};

class ProductA2 : public ProductA {
public:
    bool sell(int price) const override { return price > 2000; }
};

class ProductB {
public:
    virtual ~ProductB() = default;
    virtual bool buy(int price) const = 0;
};

class ProductB1 : public ProductB {
public:
    bool buy(int price) const override { return price < 1000; }
};

class ProductB2 : public ProductB {
public:
    bool buy(int price) const override { return price < 2000; }
};

class AbstractFactory {
public:
    virtual std::unique_ptr<ProductA> make_a() const = 0;
    virtual std::unique_ptr<ProductB> make_b() const = 0;
};

class Factory1 : public AbstractFactory {
public:
    std::unique_ptr<ProductA> make_a() const override
    {
        return std::make_unique<ProductA1>();
    }

    std::unique_ptr<ProductB> make_b() const override
    {
        return std::make_unique<ProductB1>();
    }
};

class Factory2 : public AbstractFactory {
public:
    std::unique_ptr<ProductA> make_a() const override
    {
        return std::make_unique<ProductA2>();
    }

    std::unique_ptr<ProductB> make_b() const override
    {
        return std::make_unique<ProductB2>();
    }
};
} // namespace t00020
} // namespace clanguml
