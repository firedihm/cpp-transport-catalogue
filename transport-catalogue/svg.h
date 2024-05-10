#pragma once

#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace svg {

struct Point {
    Point() = default;
    Point(double x, double y) : x(x) , y(y) {}
    
    double x = 0.0;
    double y = 0.0;
};

// ---------- Object ------------------

struct RenderContext;

class Object {
public:
    virtual ~Object() = default;
    
    void Render(const RenderContext& context) const;
    
private:
    virtual void RenderObject(const RenderContext& context) const = 0;
};

// ---------- Object::PathProps ------------------

struct Rgb {
    Rgb() = default;
    Rgb(uint8_t red, uint8_t green, uint8_t blue) : red(red), green(green), blue(blue) {}
    
    uint8_t red = 0, green = 0, blue = 0;
};

struct Rgba final : public Rgb {
    Rgba() = default;
    Rgba(uint8_t red, uint8_t green, uint8_t blue, double opacity) : Rgb(red, green, blue), opacity(opacity) {}
    
    double opacity = 1.0;
};

using Color = std::variant<std::monostate, std::string, svg::Rgb, svg::Rgba>;
std::ostream& operator<<(std::ostream& os, const Color& color);

enum class StrokeLineCap {
    BUTT,
    ROUND,
    SQUARE,
};
std::ostream& operator<<(std::ostream& os, const StrokeLineCap& value);

enum class StrokeLineJoin {
    ARCS,
    BEVEL,
    MITER,
    MITER_CLIP,
    ROUND,
};
std::ostream& operator<<(std::ostream& os, const StrokeLineJoin& value);

template <typename Owner>
class PathProps {
public:
    // эти методы вынести в .cpp не получается, в отличие от RenderAttrs...
    Owner& SetFillColor(Color color) { return fill_color_ = std::move(color), AsOwner(); }
    Owner& SetStrokeColor(Color color) { return stroke_color_ = std::move(color), AsOwner(); }
    Owner& SetStrokeWidth(double width) { return width_ = width, AsOwner(); }
    Owner& SetStrokeLineCap(StrokeLineCap line_cap) { return line_cap_ = line_cap, AsOwner(); }
    Owner& SetStrokeLineJoin(StrokeLineJoin line_join) { return line_join_ = line_join, AsOwner(); }
    
protected:
    ~PathProps() = default;
    
    void RenderAttrs(std::ostream& out) const;
    
private:
    inline Owner& AsOwner() { return static_cast<Owner&>(*this); }
    
    std::optional<Color> fill_color_;
    std::optional<Color> stroke_color_;
    std::optional<double> width_;
    std::optional<StrokeLineCap> line_cap_;
    std::optional<StrokeLineJoin> line_join_;
};

// ---------- Object::Circle ------------------

class Circle final : public Object, public PathProps<Circle> {
public:
    Circle& SetCenter(Point center);
    Circle& SetRadius(double radius);
    
private:
    void RenderObject(const RenderContext& context) const override;
    
    Point center_;
    double radius_ = 1.0;
};

// ---------- Object::Polyline ------------------

class Polyline final : public Object, public PathProps<Polyline> {
public:
    Polyline& AddPoint(Point point);
    Polyline& ReserveCapacity(uint capacity);
    
private:
    void RenderObject(const RenderContext& context) const override;
    
    std::vector<Point> points_;
};

// ---------- Object::Text ------------------

class Text final : public Object, public PathProps<Text> {
public:
    Text& SetPosition(Point pos);
    Text& SetOffset(Point offset);
    Text& SetFontSize(uint32_t size);
    Text& SetFontFamily(std::string font_family);
    Text& SetFontWeight(std::string font_weight);
    Text& SetData(std::string data);
    
private:
    void RenderObject(const RenderContext& context) const override;
    
    Point pos_;
    Point offset_;
    uint32_t size_ = 1;
    std::string font_family_;
    std::string font_weight_;
    std::string data_;
};

// ---------- Document ------------------
// до сих пор не понимаю, зачем это нужно
class ObjectContainer {
public:
    template <typename Obj>
    void Add(Obj obj) { AddPtr(std::make_unique<Obj>(std::move(obj))); }
    virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
    
protected:
    ~ObjectContainer() = default;
};

class Document final : public ObjectContainer {
public:
    void AddPtr(std::unique_ptr<Object>&& obj) override;
    
    void Render(std::ostream& out, int step, int indent) const;
    
private:
    std::vector<std::unique_ptr<Object>> objects_;
};

// namespace shape {
// ---------- Drawable ------------------

class Drawable {
public:
    virtual ~Drawable() = default;
    
    virtual void Draw(ObjectContainer& container) const = 0;
};

// ---------- Drawable::Triangle ------------------

class Triangle : public Drawable {
public:
    Triangle(Point p1, Point p2, Point p3)
        : p1_(p1), p2_(p2), p3_(p3) {}
    
    void Draw(ObjectContainer& container) const override;
    
private:
    Point p1_, p2_, p3_;
};

// ---------- Drawable::Star ------------------

class Star : public Drawable {
public:
    Star(Point center, double outer_radius, double inner_radius, int num_rays)
        : center_(center), outer_radius_(outer_radius), inner_radius_(inner_radius), num_rays_(num_rays) {}
    
    void Draw(svg::ObjectContainer& container) const override;
    
private:
    Point center_;
    double outer_radius_;
    double inner_radius_;
    int num_rays_;
};

// ---------- Drawable::Snowman ------------------

class Snowman : public Drawable {
public:
    Snowman(Point head_center, double head_radius)
        : head_center_(head_center), head_radius_(head_radius) {}
    
    void Draw(svg::ObjectContainer& container) const override;
    
private:
    Point head_center_;
    double head_radius_;
};
// } // namespace shape

}  // namespace svg
