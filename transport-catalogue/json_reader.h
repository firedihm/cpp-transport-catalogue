#pragma once

#include "json.h"
#include "map_renderer.h"
#include "transport_router.h"

namespace json {

class JsonReader {
private:
    using MapRenderer = std::unique_ptr<render::MapRenderer>;
    using TransportRouter = std::unique_ptr<router::TransportRouter>;
    
public:
    JsonReader(catalogue::TransportCatalogue& catalogue, const Document& input, std::ostream& output)
        : catalogue_(catalogue), input_(input), output_(output) {}
    
    void ProcessBaseRequests();
    void PrintStats(int step = 4, int indent = 0);
    void RenderMap(int step = 0, int indent = 4);
    
private:
    const Document ProcessStatRequests();
    
    Dict MakeBusResponse(const Dict& request);
    Dict MakeStopResponse(const Dict& request);
    static Dict MakeMapResponse(const Dict& request, const MapRenderer& renderer);
    static Dict MakeRouteResponse(const Dict& request, const TransportRouter& router);
    
    static geo::Coordinates ParseCoordinates(const Dict& request);
    static std::vector<std::pair<std::string_view, int>> ParseDistances(const Dict& request);
    static std::vector<std::string_view> ParseRoute(const Dict& request);
    static svg::Color ParseColor(const Node& node);
    static render::RenderSettings ParseRenderSettings(const Dict& settings);
    static router::RoutingSettings ParseRouteSettings(const Dict& settings);
    
    catalogue::TransportCatalogue& catalogue_;
    const Document& input_;
    std::ostream& output_;
};

} // namespace json
