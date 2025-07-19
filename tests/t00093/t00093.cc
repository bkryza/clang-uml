#include <memory>
#include <string>
#include <vector>

namespace clanguml {
namespace t00093 {

// Bridge pattern implementation: Drawing system with multiple renderers

// Implementor interface
class Renderer {
public:
    virtual ~Renderer() = default;
    virtual void render_circle(double x, double y, double radius) = 0;
    virtual void render_rectangle(
        double x, double y, double width, double height) = 0;
};

// Concrete Implementors
class VectorRenderer : public Renderer {
public:
    void render_circle(double x, double y, double radius) override { }
    void render_rectangle(
        double x, double y, double width, double height) override
    {
    }

    void set_stroke_width(double width) { stroke_width = width; }
    void set_fill_color(const std::string &color) { fill_color = color; }

private:
    double stroke_width{1.0};
    std::string fill_color{"black"};
};

class RasterRenderer : public Renderer {
public:
    void render_circle(double x, double y, double radius) override { }
    void render_rectangle(
        double x, double y, double width, double height) override
    {
    }

    void set_dpi(int dpi) { this->dpi = dpi; }
    void set_anti_aliasing(bool enabled) { anti_aliasing = enabled; }

private:
    int dpi{72};
    bool anti_aliasing{true};
};

// Abstraction
class Shape {
public:
    explicit Shape(std::shared_ptr<Renderer> renderer)
        : renderer_bridge_(std::move(renderer))
    {
    }

    virtual ~Shape() = default;
    virtual void draw() = 0;
    virtual void move(double dx, double dy) = 0;

protected:
    std::shared_ptr<Renderer> renderer_bridge_;
    double x_{0.0};
    double y_{0.0};
};

// Refined Abstractions
class Circle : public Shape {
public:
    Circle(std::shared_ptr<Renderer> renderer, double radius)
        : Shape(std::move(renderer))
        , radius_(radius)
    {
    }

    void draw() override { renderer_bridge_->render_circle(x_, y_, radius_); }

    void move(double dx, double dy) override
    {
        x_ += dx;
        y_ += dy;
    }

    void set_radius(double radius) { radius_ = radius; }
    double get_radius() const { return radius_; }

private:
    double radius_;
};

class Rectangle : public Shape {
public:
    Rectangle(std::shared_ptr<Renderer> renderer, double width, double height)
        : Shape(std::move(renderer))
        , width_(width)
        , height_(height)
    {
    }

    void draw() override
    {
        renderer_bridge_->render_rectangle(x_, y_, width_, height_);
    }

    void move(double dx, double dy) override
    {
        x_ += dx;
        y_ += dy;
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

// Client code demonstrating the Bridge pattern
class DrawingApplication {
public:
    DrawingApplication(std::shared_ptr<Renderer> renderer)
        : renderer_(std::move(renderer))
    {
    }

    void create_circle(double radius)
    {
        auto circle = std::make_unique<Circle>(renderer_, radius);
        shapes_.push_back(std::move(circle));
    }

    void create_rectangle(double width, double height)
    {
        auto rectangle = std::make_unique<Rectangle>(renderer_, width, height);
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
    std::shared_ptr<Renderer> renderer_;
    std::vector<std::unique_ptr<Shape>> shapes_;
};

} // namespace t00093
} // namespace clanguml