#include <concepts>
#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace clanguml {
namespace t00099 {

template <typename T>
concept Renderer = requires(
    T &t, double x, double y, double radius, double width, double height) {
    t.render_circle(x, y, radius);
    t.render_rectangle(x, y, width, height);
};

class VectorRenderer {
public:
    void render_circle(double x, double y, double radius) { }
    void render_rectangle(double x, double y, double width, double height) { }

    void set_stroke_width(double width) { stroke_width = width; }
    void set_fill_color(const std::string &color) { fill_color = color; }

private:
    double stroke_width{1.0};
    std::string fill_color{"black"};
};

class RasterRenderer {
public:
    void render_circle(double x, double y, double radius) { }
    void render_rectangle(double x, double y, double width, double height) { }

    void set_dpi(int dpi) { this->dpi = dpi; }
    void set_anti_aliasing(bool enabled) { anti_aliasing = enabled; }

private:
    int dpi{72};
    bool anti_aliasing{true};
};

template <Renderer R> class Shape {
public:
    explicit Shape(R renderer)
        : renderer_bridge_(std::move(renderer))
    {
    }

    virtual ~Shape() = default;
    virtual void draw() = 0;
    virtual void move(double dx, double dy) = 0;

protected:
    R renderer_bridge_;
    double x_{0.0};
    double y_{0.0};
};

template <Renderer R> class Circle : public Shape<R> {
public:
    Circle(R renderer, double radius)
        : Shape<R>(std::move(renderer))
        , radius_(radius)
    {
    }

    void draw() override
    {
        this->renderer_bridge_.render_circle(this->x_, this->y_, radius_);
    }

    void move(double dx, double dy) override
    {
        this->x_ += dx;
        this->y_ += dy;
    }

    void set_radius(double radius) { radius_ = radius; }
    double get_radius() const { return radius_; }

private:
    double radius_;
};

template <Renderer R> class Rectangle : public Shape<R> {
public:
    Rectangle(R renderer, double width, double height)
        : Shape<R>(std::move(renderer))
        , width_(width)
        , height_(height)
    {
    }

    void draw() override
    {
        this->renderer_bridge_.render_rectangle(
            this->x_, this->y_, width_, height_);
    }

    void move(double dx, double dy) override
    {
        this->x_ += dx;
        this->y_ += dy;
    }

    void set_dimensions(double width, double height)
    {
        width_ = width;
        height_ = height;
    }

    double get_width() const { return width_; }
    double get_height() const { return height_; }

private:
    double width_;
    double height_;
};

template <Renderer R> class DrawingApplication {
public:
    DrawingApplication(R renderer)
        : renderer_(std::move(renderer))
    {
    }

    void create_circle(double radius)
    {
        auto circle = std::make_unique<Circle<R>>(renderer_, radius);
        shapes_.push_back(std::move(circle));
    }

    void create_rectangle(double width, double height)
    {
        auto rectangle =
            std::make_unique<Rectangle<R>>(renderer_, width, height);
        shapes_.push_back(std::move(rectangle));
    }

    void draw_all()
    {
        for (const auto &shape : shapes_) {
            shape->draw();
        }
    }

    void move_all(double dx, double dy)
    {
        for (const auto &shape : shapes_) {
            shape->move(dx, dy);
        }
    }

private:
    R renderer_;
    std::vector<std::unique_ptr<Shape<R>>> shapes_;
};

using VectorApp = DrawingApplication<VectorRenderer>;
using RasterApp = DrawingApplication<RasterRenderer>;

struct App {
    std::variant<VectorApp, RasterApp> impl;

    static App make_vector_app() { return App{.impl = VectorRenderer{}}; }

    static App make_raster_app() { return App{.impl = RasterRenderer{}}; }
};

int main()
{
    auto app = App::make_vector_app();

    return 0;
}

} // namespace t00099
} // namespace clanguml