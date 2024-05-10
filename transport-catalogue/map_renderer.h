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

void RenderMap(RenderDetails&& details, const catalogue::TransportCatalogue& catalogue, std::ostream& output, int step = 0, int indent = 4);

} // namespace render
