#pragma once

#include "svg.h"
#include "transport_catalogue.h"

namespace render {

struct RenderDetails {
    double width = 0.0;
    double height = 0.0;
    double padding = 0.0;
    
    double line_width = 0.0;
    double stop_radius = 0.0;
    
    int bus_label_font_size = 0;
    svg::Point bus_label_offset;
    
    int stop_label_font_size = 0;
    svg::Point stop_label_offset;
    
    svg::Color underlayer_color;
    double underlayer_width = 0.0;
    
    std::vector<svg::Color> colors;
};

class MapRenderer {
public:
    MapRenderer(RenderDetails&& details, const catalogue::TransportCatalogue& catalogue, std::ostream& output)
        : details_(std::move(details)), catalogue_(catalogue), output_(output) {
        
        InitScalingFactors();
        InitSortedLists();
        RenderMap(); // можно просто рисовать отсюда
    }
    MapRenderer(const MapRenderer&) = delete;
    MapRenderer(MapRenderer&&) = delete;
    
    MapRenderer& operator=(const MapRenderer&) = delete;
    MapRenderer& operator=(MapRenderer&&) = delete;
    
private:
    void InitScalingFactors();
    void InitSortedLists();
    
    void RenderMap();
    void DrawBusTraces();
    void DrawBusNames();
    void DrawStops();
    void DrawStopNames();
    
    svg::Point TransformCoordsToScreenSpace(const geo::Coordinates& coords);
    
    const RenderDetails details_;
    const catalogue::TransportCatalogue& catalogue_;
    svg::Document document_;
    std::ostream& output_;
    
    double min_lng_;
    double max_lat_;
    double zoom_;
    
    std::vector<const catalogue::Stop*> sorted_stops_;
    std::vector<const catalogue::Bus*> sorted_buses_;
};

void RenderMap(RenderDetails&& details, const catalogue::TransportCatalogue& catalogue, std::ostream& output, int step = 0, int indent = 4);

} // namespace render
