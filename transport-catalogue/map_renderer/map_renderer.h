#pragma once

#include "svg.h"
#include "transport_catalogue.h"

namespace render {

struct RenderSettings {
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
    MapRenderer(RenderSettings&& settings, const catalogue::TransportCatalogue& catalogue)
        : settings_(std::move(settings)), catalogue_(catalogue) {
        
        InitScalingFactors();
        InitSortedLists();
    }
    MapRenderer(const MapRenderer&) = delete;
    MapRenderer(MapRenderer&&) = delete;
    
    MapRenderer& operator=(const MapRenderer&) = delete;
    MapRenderer& operator=(MapRenderer&&) = delete;
    
    void RenderMap(std::ostream& output, int step, int indent);
    
private:
    void InitScalingFactors();
    void InitSortedLists();
    
    void DrawBusTraces();
    void DrawBusNames();
    void DrawStops();
    void DrawStopNames();
    
    svg::Point TransformCoordsToScreenSpace(const geo::Coordinates& coords);
    
    const RenderSettings settings_;
    const catalogue::TransportCatalogue& catalogue_;
    svg::Document document_;
    
    double min_lng_;
    double max_lat_;
    double zoom_;
    
    std::vector<const catalogue::Stop*> sorted_stops_;
    std::vector<const catalogue::Bus*> sorted_buses_;
};

} // namespace render
