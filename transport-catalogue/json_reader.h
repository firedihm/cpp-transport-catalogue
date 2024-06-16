#pragma once

#include "json.h"
#include "transport_catalogue.h"

namespace json {

class JsonReader {
public:
    JsonReader(catalogue::TransportCatalogue& catalogue, const Document& input, std::ostream& output)
        : catalogue_(catalogue), input_(input), output_(output) {}
    
    void ProcessBaseRequests();
    void PrintStats(int step = 4, int indent = 0);
    void RenderMap(int step = 0, int indent = 4);
    
private:
    Dict MakeBusResponse(const Dict& request);
    Dict MakeStopResponse(const Dict& request);
    
    /* методы ниже опираются на вспомогательные объекты, которые лениво инициализируются в ProcessStatRequests()
     * static Dict MakeMapResponse(const Dict& request, const MapRenderer& renderer);
     * static Dict MakeRouteResponse(const Dict& request, const TransportRouter& router);
     */
    
    const Document ProcessStatRequests();
    
    catalogue::TransportCatalogue& catalogue_;
    const Document& input_;
    std::ostream& output_;
};

} // namespace json
