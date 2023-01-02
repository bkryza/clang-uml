#include <memory>
#include <tuple>
#include <type_traits>

namespace clanguml {
namespace t00027 {

class Shape {
public:
    virtual void display() = 0;
    virtual ~Shape() = default;
};

template <template <typename> class... T>
class Line : public Shape, public T<Line<>>... {
public:
    void display() override
    {
        std::apply([](auto &&...x) { (x.display(), ...); },
            std::forward_as_tuple(T<Line<>>()...));
    }
};

template <template <typename> class... T>
class Text : public Shape, public T<Text<>>... {
public:
    void display() override
    {
        std::apply([](auto &&...x) { (x.display(), ...); },
            std::forward_as_tuple(T<Text<>>()...));
    }
};

struct ShapeDecorator {
    virtual void display() = 0;
};

template <typename T> class Color : public ShapeDecorator {
public:
    void display() override { }
};

template <typename T> class Weight : public ShapeDecorator {
public:
    void display() override { }
};

struct Window {
    Line<Color, Weight> border;
    Line<Color> divider;
    Text<Color, Weight> title;
    Text<Color> description;
};
} // namespace t00027
} // namespace clanguml
