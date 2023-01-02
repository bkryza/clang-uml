#include <memory>

namespace clanguml {
namespace t00023 {

class Strategy {
public:
    virtual ~Strategy() = default;
    virtual void algorithm() = 0;
};

class StrategyA : public Strategy {
public:
    void algorithm() override { }
};

class StrategyB : public Strategy {
public:
    void algorithm() override { }
};

class StrategyC : public Strategy {
public:
    void algorithm() override { }
};

class Context {
public:
    Context(std::unique_ptr<Strategy> strategy)
        : m_strategy(std::move(strategy))
    {
    }

    void apply() { m_strategy->algorithm(); }

private:
    std::unique_ptr<Strategy> m_strategy;
};
} // namespace t00023
} // namespace clanguml
