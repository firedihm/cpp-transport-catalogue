#include "svg.h"

#define _USE_MATH_DEFINES
#include <cmath>
#include <iomanip>

using namespace std::literals;

namespace svg {

// ---------- Object ------------------

struct RenderContext {
    RenderContext(std::ostream& out, int step, int indent) : out(out), step(step), indent(indent) {}
    
    RenderContext Indented() const { return RenderContext(out, step, indent + step); }
    void RenderIndent() const { out << std::setw(indent) << (indent ? " "sv : ""sv); }
    
    std::ostream& out;
    int step;
    int indent;
};

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();
    
    // Делегируем вывод тега своим подклассам
    RenderObject(context);
    
    context.out << std::endl;
}

// ---------- Object::PathProps ------------------

namespace detail {
void RenderColor(std::ostream& os, const std::monostate) {
    os << "none"sv;
}

void RenderColor(std::ostream& os, const std::string value) {
    os << value;
}

void RenderColor(std::ostream& os, const Rgb& value) {
    os << "rgb("sv
       << std::to_string(value.red) << ','
       << std::to_string(value.green) << ','
       << std::to_string(value.blue) << ')';
}

void RenderColor(std::ostream& os, const Rgba& value) {
    os << "rgba("sv
       << std::to_string(value.red) << ','
       << std::to_string(value.green) << ','
       << std::to_string(value.blue) << ','
       << value.opacity << ')';
}
} // namespace details

std::ostream& operator<<(std::ostream& os, const Color& color) {
    std::visit([&os](auto value) { detail::RenderColor(os, value); }, color);
    return os;
}

std::ostream& operator<<(std::ostream& os, const StrokeLineCap& value) {
    switch (value) {
        case StrokeLineCap::BUTT:
            os << "butt"sv;
            break;
        case StrokeLineCap::ROUND:
            os << "round"sv;
            break;
        case StrokeLineCap::SQUARE:
            os << "square"sv;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const StrokeLineJoin& value) {
    switch (value) {
        case StrokeLineJoin::ARCS:
            os << "arcs"sv;
            break;
        case StrokeLineJoin::BEVEL:
            os << "bevel"sv;
            break;
        case StrokeLineJoin::MITER:
            os << "miter"sv;
            break;
        case StrokeLineJoin::MITER_CLIP:
            os << "miter-clip"sv;
            break;
        case StrokeLineJoin::ROUND:
            os << "round"sv;
    }
    return os;
}

template <typename Owner>
void PathProps<Owner>::RenderAttrs(std::ostream& out) const {
    if (fill_color_) { out << " fill=\""sv << *fill_color_ << "\""sv; }
    if (stroke_color_) { out << " stroke=\""sv << *stroke_color_ << "\""sv; }
    if (width_) { out << " stroke-width=\""sv << *width_ << "\""sv; }
    if (line_cap_) { out << " stroke-linecap=\""sv << *line_cap_ << "\""sv; }
    if (line_join_) { out << " stroke-linejoin=\""sv << *line_join_ << "\""sv; }
}

// ---------- Object::Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = std::move(center);
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv
        << "r=\""sv << radius_ << "\""sv;
    RenderAttrs(context.out);
    out << "/>"sv;
}

// ---------- Object::Polyline ------------------

Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(std::move(point));
    return *this;
}

Polyline& Polyline::ReserveCapacity(uint capacity) {
    points_.reserve(capacity);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\""sv;
    if (!points_.empty()) {
        auto it = points_.begin();
        out << (*it).x << ',' << (*it).y;
        while (++it != points_.end()) {
            out << ' ' << (*it).x << ',' << (*it).y;
        }
    }
    out << "\""sv;
    RenderAttrs(out);
    out << "/>"sv;
}

// ---------- Object::Text ------------------

Text& Text::SetPosition(Point pos) {
    pos_ = std::move(pos);
    return *this;
}

Text& Text::SetOffset(Point offset) {
    offset_ = std::move(offset);
    return *this;
}

Text& Text::SetFontSize(uint32_t size) {
    size_ = size;
    return *this;
}

Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = std::move(font_family);
    return *this;
}

Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = std::move(font_weight);
    return *this;
}

Text& Text::SetData(std::string data) {
    data_ = std::move(data);
    return *this;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<text"sv;
    RenderAttrs(out);
    out << " x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\" "sv
        << "dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" "sv
        << "font-size=\""sv << size_ << "\""sv;
    if (!font_family_.empty()) {
        out << " font-family=\""sv << font_family_ << "\""sv;
    }
    if (!font_weight_.empty()) {
        out << " font-weight=\""sv << font_weight_ << "\""sv;
    }
    out << '>' << data_ << "</text>"sv;
}

// ---------- Document ------------------

void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_.push_back(std::move(obj));
}

void Document::Render(std::ostream& out, int step, int indent) const {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"sv
        << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n"sv;
    RenderContext ctx(out, step, indent);
    for (const auto& object : objects_) {
        object->Render(ctx);
    }
    out << "</svg>\n"sv;
}

// namespace shape {
// ---------- Drawable::Triangle ------------------

void Triangle::Draw(ObjectContainer& container) const {
    container.Add(Polyline().AddPoint(p1_).AddPoint(p2_).AddPoint(p3_).AddPoint(p1_));
}

// ---------- Drawable::Star ------------------

void Star::Draw(ObjectContainer& container) const {
    Polyline polyline;
    for (int i = 0; i <= num_rays_; ++i) {
        double angle = 2 * M_PI * (i % num_rays_) / num_rays_;
        polyline.AddPoint({center_.x + outer_radius_ * sin(angle), center_.y - outer_radius_ * cos(angle)});
        if (i == num_rays_) {
            break;
        }
        angle += M_PI / num_rays_;
        polyline.AddPoint({center_.x + inner_radius_ * sin(angle), center_.y - inner_radius_ * cos(angle)});
    }
    
    container.Add(std::move(polyline.SetFillColor("red"s).SetStrokeColor("black"s)));
}

// ---------- Drawable::Snowman ------------------

void Snowman::Draw(ObjectContainer& container) const {
    container.Add(Circle().SetCenter(Point(head_center_.x, head_center_.y + 5.0 * head_radius_))
                          .SetRadius(2.0 * head_radius_)
                          .SetFillColor(Rgb(240,240,240))
                          .SetStrokeColor("black"s));
    container.Add(Circle().SetCenter(Point(head_center_.x, head_center_.y + 2.0 * head_radius_))
                          .SetRadius(1.5 * head_radius_)
                          .SetFillColor(Rgb(240,240,240))
                          .SetStrokeColor("black"s));
    container.Add(Circle().SetCenter(head_center_)
                          .SetRadius(head_radius_)
                          .SetFillColor(Rgb(240,240,240))
                          .SetStrokeColor("black"s));
}
// } // namespace shape

}  // namespace svg
