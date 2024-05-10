#pragma once

#include "json.h"
#include "map_renderer.h"
#include "transport_catalogue.h"

namespace json {

class JsonReader {
public:
    JsonReader(catalogue::TransportCatalogue& catalogue, const Document& input, std::ostream& output)
        : catalogue_(catalogue), input_(input), output_(output) {}
    
    void ProcessBaseRequests();
    const Document ProcessStatRequests();
    void Print(const Document& response, int step = 4, int indent = 0);
    void RenderMap(int step = 0, int indent = 4);
    
private:
    Dict MakeBusResponse(const Dict& request);
    Dict MakeStopResponse(const Dict& request);
    Dict MakeMapResponse(const Dict& request);
    
    static geo::Coordinates ParseCoordinates(const Dict& request);
    static std::vector<std::pair<std::string_view, int>> ParseDistances(const Dict& request);
    static std::vector<std::string_view> ParseRoute(const Dict& request);
    static svg::Color ParseColor(const Node& node);
    static render::RenderDetails ParseRenderDetails(const Dict& settings);
    
    catalogue::TransportCatalogue& catalogue_;
    const Document& input_;
    std::ostream& output_;
};

} // namespace json
