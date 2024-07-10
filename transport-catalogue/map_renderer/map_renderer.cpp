#include "map_renderer.h"

#include <algorithm>
#include <optional>

using namespace catalogue;

namespace render {

inline const double EPSILON = 1e-6;
bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

void MapRenderer::InitScalingFactors() {
    const auto& /* catalogue::MinMaxCoords */ [min, max] = catalogue_.GetMinMaxCoords();
    min_lng_ = min.lng;
    max_lat_ = max.lat;
    
    // Вычисляем коэффициент масштабирования вдоль координаты x
    std::optional<double> width_zoom;
    if (!IsZero(max.lng - min_lng_)) {
        width_zoom = (settings_.width - 2 * settings_.padding) / (max.lng - min_lng_);
    }
    
    // Вычисляем коэффициент масштабирования вдоль координаты y
    std::optional<double> height_zoom;
    if (!IsZero(max_lat_ - min.lat)) {
        height_zoom = (settings_.height - 2 * settings_.padding) / (max_lat_ - min.lat);
    }
    
    if (width_zoom && height_zoom) {
        zoom_ = std::min(*width_zoom, *height_zoom);
    } else if (width_zoom) {
        zoom_ = *width_zoom;
    } else if (height_zoom) {
        zoom_ = *height_zoom;
    } else {
        zoom_ = 0.0;
    }
}

void MapRenderer::InitSortedLists() {
    // получим и отсортируем представление элементов каталога для их упорядоченной отрисовки; сначала остановки...
    std::vector<const Stop*> sorted_stops;
    sorted_stops.reserve(catalogue_.GetStopsData().size());
    for (const Stop& stop : catalogue_.GetStopsData()) {
        sorted_stops.push_back(&stop);
    }
    std::sort(sorted_stops.begin(), sorted_stops.end(), [](const Stop* lhs, const Stop* rhs) {
        return lhs->name < rhs->name;
    });
    sorted_stops_ = std::move(sorted_stops);
    
    // ...потом автобусы
    std::vector<const Bus*> sorted_buses;
    sorted_buses.reserve(catalogue_.GetBusesData().size());
    for (const Bus& bus : catalogue_.GetBusesData()) {
        sorted_buses.push_back(&bus);
    }
    std::sort(sorted_buses.begin(), sorted_buses.end(), [](const Bus* lhs, const Bus* rhs) {
        return lhs->name < rhs->name;
    });
    sorted_buses_ = std::move(sorted_buses);
}

void MapRenderer::RenderMap(std::ostream& output, int step, int indent) {
    DrawBusTraces();
    DrawBusNames();
    DrawStops();
    DrawStopNames();
    document_.Render(output, step, indent);
}

void MapRenderer::DrawBusTraces() {
    uint color_id = 0;
    for (const Bus* bus : sorted_buses_) {
        if (bus->route.empty()) {
            continue;
        }
        
        svg::Polyline route;
        route.ReserveCapacity(bus->route.size());
        for (const Stop* stop : bus->route) {
            route.AddPoint(TransformCoordsToScreenSpace(stop->coords));
        }
        
        document_.Add(route.SetFillColor("none")
                           .SetStrokeColor(settings_.colors[color_id++ % settings_.colors.size()])
                           .SetStrokeWidth(settings_.line_width)
                           .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                           .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND));
    }
}

void MapRenderer::DrawBusNames() {
    uint color_id = 0;
    for (const Bus* bus : sorted_buses_) {
        if (bus->route.empty()) {
            continue;
        }
        
        std::vector<const Stop*> final_stops;
        final_stops.reserve(2);
        final_stops.push_back(bus->route.front());
        if (bus->type == RouteType::PENDULUM && bus->route[0] != bus->route[bus->route.size() / 2]) {
            final_stops.push_back(bus->route[bus->route.size() / 2]);
        }
        
        for (const Stop* stop : final_stops) {
            // background
            document_.Add(svg::Text().SetFillColor(settings_.underlayer_color)
                                     .SetStrokeColor(settings_.underlayer_color)
                                     .SetStrokeWidth(settings_.underlayer_width)
                                     .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                                     .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                                     .SetPosition(TransformCoordsToScreenSpace(stop->coords))
                                     .SetOffset(settings_.bus_label_offset)
                                     .SetFontSize(settings_.bus_label_font_size)
                                     .SetFontFamily("Verdana")
                                     .SetFontWeight("bold")
                                     .SetData(bus->name));
            
            // foreground
            document_.Add(svg::Text().SetFillColor(settings_.colors[color_id])
                                     .SetPosition(TransformCoordsToScreenSpace(stop->coords))
                                     .SetOffset(settings_.bus_label_offset)
                                     .SetFontSize(settings_.bus_label_font_size)
                                     .SetFontFamily("Verdana")
                                     .SetFontWeight("bold")
                                     .SetData(bus->name));
        }
        ++color_id %= settings_.colors.size();
    }
}

void MapRenderer::DrawStops() {
    for (const Stop* stop : sorted_stops_) {
        if (stop->passing_buses.empty()) {
            continue;
        }
        
        document_.Add(svg::Circle().SetFillColor("white")
                                   .SetCenter(TransformCoordsToScreenSpace(stop->coords))
                                   .SetRadius(settings_.stop_radius));
    }
}

void MapRenderer::DrawStopNames() {
    for (const Stop* stop : sorted_stops_) {
        if (stop->passing_buses.empty()) {
            continue;
        }
        
        // background
        document_.Add(svg::Text().SetFillColor(settings_.underlayer_color)
                                 .SetStrokeColor(settings_.underlayer_color)
                                 .SetStrokeWidth(settings_.underlayer_width)
                                 .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                                 .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                                 .SetPosition(TransformCoordsToScreenSpace(stop->coords))
                                 .SetOffset(settings_.stop_label_offset)
                                 .SetFontSize(settings_.stop_label_font_size)
                                 .SetFontFamily("Verdana")
                                 .SetData(stop->name));
        
        // foreground
        document_.Add(svg::Text().SetFillColor("black")
                                 .SetPosition(TransformCoordsToScreenSpace(stop->coords))
                                 .SetOffset(settings_.stop_label_offset)
                                 .SetFontSize(settings_.stop_label_font_size)
                                 .SetFontFamily("Verdana")
                                 .SetData(stop->name));
    }
}

svg::Point MapRenderer::TransformCoordsToScreenSpace(const geo::Coordinates& coords) {
    svg::Point point;
    point.x = (coords.lng - min_lng_) * zoom_ + settings_.padding;
    point.y = (max_lat_ - coords.lat) * zoom_ + settings_.padding;
    return point;
}

} // namespace render
