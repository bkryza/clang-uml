#include <concepts>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace clanguml {
namespace t00096 {

// This concept defining the requirements for a cloneable type
template <typename T>
concept Cloneable = requires(const T &obj) {
    { obj.clone() } -> std::same_as<std::unique_ptr<T>>;
    typename T::clone_type;
    std::is_copy_constructible_v<T>;
};

// Concept for drawable objects
template <typename T>
concept Drawable = requires(const T &obj) {
    { obj.draw() } -> std::same_as<void>;
    { obj.get_area() } -> std::convertible_to<double>;
    { obj.get_type_name() } -> std::convertible_to<std::string>;
};

template <typename T>
concept CloneableDrawable = Cloneable<T> && Drawable<T>;

template <typename Derived> class CloneableBase {
public:
    using clone_type = Derived;

    std::unique_ptr<Derived> clone() const
    {
        return std::make_unique<Derived>(static_cast<const Derived &>(*this));
    }

    virtual ~CloneableBase() = default;
};

// Abstract shape interface (without virtual clone)
class Shape {
public:
    virtual ~Shape() = default;
    virtual void draw() const = 0;
    virtual double get_area() const = 0;
    virtual std::string get_type_name() const = 0;

    void set_position(double x, double y)
    {
        x_ = x;
        y_ = y;
    }
    double get_x() const { return x_; }
    double get_y() const { return y_; }

protected:
    double x_{0.0};
    double y_{0.0};
};

// Concrete shape: Circle
class Circle : public Shape, public CloneableBase<Circle> {
public:
    Circle() = default;

    explicit Circle(double radius)
        : radius_(radius)
    {
    }

    Circle(double radius, double x, double y)
        : radius_(radius)
    {
        x_ = x;
        y_ = y;
    }

    // Copy constructor for cloning
    Circle(const Circle &other)
        : radius_(other.radius_)
    {
        x_ = other.x_;
        y_ = other.y_;
    }

    void draw() const override { }

    double get_area() const override { return 3.14159 * radius_ * radius_; }

    std::string get_type_name() const override { return "Circle"; }

    void set_radius(double radius) { radius_ = radius; }
    double get_radius() const { return radius_; }

private:
    double radius_{1.0};
};

// Concrete shape: Rectangle
class Rectangle : public Shape, public CloneableBase<Rectangle> {
public:
    Rectangle() = default;

    Rectangle(double width, double height)
        : width_(width)
        , height_(height)
    {
    }

    Rectangle(double width, double height, double x, double y)
        : width_(width)
        , height_(height)
    {
        x_ = x;
        y_ = y;
    }

    // Copy constructor for cloning
    Rectangle(const Rectangle &other)
        : width_(other.width_)
        , height_(other.height_)
    {
        x_ = other.x_;
        y_ = other.y_;
    }

    void draw() const override { }

    double get_area() const override { return width_ * height_; }

    std::string get_type_name() const override { return "Rectangle"; }

    void set_dimensions(double width, double height)
    {
        width_ = width;
        height_ = height;
    }

    double get_width() const { return width_; }
    double get_height() const { return height_; }

private:
    double width_{1.0};
    double height_{1.0};
};

// Concrete shape: Triangle
class Triangle : public Shape, public CloneableBase<Triangle> {
public:
    Triangle() = default;

    Triangle(double base, double height)
        : base_(base)
        , height_(height)
    {
    }

    Triangle(double base, double height, double x, double y)
        : base_(base)
        , height_(height)
    {
        x_ = x;
        y_ = y;
    }

    // Copy constructor for cloning
    Triangle(const Triangle &other)
        : base_(other.base_)
        , height_(other.height_)
    {
        x_ = other.x_;
        y_ = other.y_;
    }

    void draw() const override { }

    double get_area() const override { return 0.5 * base_ * height_; }

    std::string get_type_name() const override { return "Triangle"; }

    void set_dimensions(double base, double height)
    {
        base_ = base;
        height_ = height;
    }

    double get_base() const { return base_; }
    double get_height() const { return height_; }

private:
    double base_{1.0};
    double height_{1.0};
};

// Template-based prototype registry
template <CloneableDrawable T> class PrototypeRegistry {
public:
    void register_prototype(const std::string &name, const T &prototype)
    {
        prototypes_[name] = std::make_unique<T>(prototype);
    }

    std::unique_ptr<T> create_clone(const std::string &name) const
    {
        auto it = prototypes_.find(name);
        if (it != prototypes_.end()) {
            return it->second->clone();
        }
        return nullptr;
    }

    bool has_prototype(const std::string &name) const
    {
        return prototypes_.find(name) != prototypes_.end();
    }

    std::vector<std::string> get_prototype_names() const
    {
        std::vector<std::string> names;
        for (const auto &pair : prototypes_) {
            names.push_back(pair.first);
        }
        return names;
    }

    size_t size() const { return prototypes_.size(); }

private:
    std::unordered_map<std::string, std::unique_ptr<T>> prototypes_;
};

// Template factory using prototype registries
template <CloneableDrawable... Types> class ShapeFactory {
public:
    ShapeFactory() { setup_default_prototypes(); }

    template <CloneableDrawable T>
    void register_prototype(const std::string &name, const T &prototype)
    {
        if constexpr (std::is_same_v<T, Circle>) {
            circle_registry_.register_prototype(name, prototype);
        }
        else if constexpr (std::is_same_v<T, Rectangle>) {
            rectangle_registry_.register_prototype(name, prototype);
        }
        else if constexpr (std::is_same_v<T, Triangle>) {
            triangle_registry_.register_prototype(name, prototype);
        }
    }

    template <CloneableDrawable T>
    std::unique_ptr<T> create_shape(const std::string &name) const
    {
        if constexpr (std::is_same_v<T, Circle>) {
            return circle_registry_.create_clone(name);
        }
        else if constexpr (std::is_same_v<T, Rectangle>) {
            return rectangle_registry_.create_clone(name);
        }
        else if constexpr (std::is_same_v<T, Triangle>) {
            return triangle_registry_.create_clone(name);
        }
        return nullptr;
    }

    std::unique_ptr<Circle> create_circle(const std::string &name) const
    {
        return circle_registry_.create_clone(name);
    }

    std::unique_ptr<Rectangle> create_rectangle(const std::string &name) const
    {
        return rectangle_registry_.create_clone(name);
    }

    std::unique_ptr<Triangle> create_triangle(const std::string &name) const
    {
        return triangle_registry_.create_clone(name);
    }

    template <CloneableDrawable T>
    bool has_prototype(const std::string &name) const
    {
        if constexpr (std::is_same_v<T, Circle>) {
            return circle_registry_.has_prototype(name);
        }
        else if constexpr (std::is_same_v<T, Rectangle>) {
            return rectangle_registry_.has_prototype(name);
        }
        else if constexpr (std::is_same_v<T, Triangle>) {
            return triangle_registry_.has_prototype(name);
        }
        return false;
    }

private:
    void setup_default_prototypes()
    {
        circle_registry_.register_prototype("default_circle", Circle(1.0));
        circle_registry_.register_prototype("large_circle", Circle(5.0));

        rectangle_registry_.register_prototype(
            "default_rectangle", Rectangle(2.0, 3.0));
        rectangle_registry_.register_prototype("square", Rectangle(2.0, 2.0));

        triangle_registry_.register_prototype(
            "default_triangle", Triangle(3.0, 4.0));
        triangle_registry_.register_prototype(
            "right_triangle", Triangle(3.0, 4.0));
    }

    PrototypeRegistry<Circle> circle_registry_;
    PrototypeRegistry<Rectangle> rectangle_registry_;
    PrototypeRegistry<Triangle> triangle_registry_;
};

// Generic cloning utility function
template <CloneableDrawable T> std::unique_ptr<T> clone_shape(const T &original)
{
    return original.clone();
}

// Template-based shape manager
template <CloneableDrawable T> class ShapeManager {
public:
    void add_prototype(const std::string &name, const T &prototype)
    {
        registry_.register_prototype(name, prototype);
    }

    std::unique_ptr<T> create_from_prototype(const std::string &name) const
    {
        return registry_.create_clone(name);
    }

    std::vector<std::unique_ptr<T>> create_multiple(
        const std::string &name, size_t count) const
    {
        std::vector<std::unique_ptr<T>> shapes;
        shapes.reserve(count);

        for (size_t i = 0; i < count; ++i) {
            auto shape = registry_.create_clone(name);
            if (shape) {
                shapes.push_back(std::move(shape));
            }
        }
        return shapes;
    }

    double calculate_total_area(
        const std::vector<std::unique_ptr<T>> &shapes) const
    {
        double total = 0.0;
        for (const auto &shape : shapes) {
            if (shape) {
                total += shape->get_area();
            }
        }
        return total;
    }

private:
    PrototypeRegistry<T> registry_;
};

// Application demonstrating template-based prototype usage
class DrawingApplication {
public:
    DrawingApplication()
        : factory_(
              std::make_unique<ShapeFactory<Circle, Rectangle, Triangle>>())
    {
    }

    std::unique_ptr<Circle> create_circle_from_template(
        const std::string &template_name) const
    {
        return factory_->create_circle(template_name);
    }

    std::unique_ptr<Rectangle> create_rectangle_from_template(
        const std::string &template_name) const
    {
        return factory_->create_rectangle(template_name);
    }

    std::unique_ptr<Triangle> create_triangle_from_template(
        const std::string &template_name) const
    {
        return factory_->create_triangle(template_name);
    }

    template <CloneableDrawable T>
    void register_custom_template(const std::string &name, const T &prototype)
    {
        factory_->register_prototype(name, prototype);
    }

    template <CloneableDrawable T>
    std::unique_ptr<T> duplicate_shape(const T &original)
    {
        return clone_shape(original);
    }

private:
    std::unique_ptr<ShapeFactory<Circle, Rectangle, Triangle>> factory_;
};

} // namespace t00096
} // namespace clanguml