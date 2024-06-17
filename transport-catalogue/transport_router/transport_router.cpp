#include "transport_router.h"

using namespace catalogue;

namespace router {

void TransportRouter::InitGraphStopEdges() {
    const std::deque<Stop>& stops = catalogue_.GetStopsData();
    
    stop_to_vertices_.reserve(stops.size());
    edge_to_response_.reserve(stops.size());
    
    Weight wait_time = static_cast<Weight>(settings_.wait_time);
    for (graph::VertexId index = 0; const Stop& stop : stops) {
        graph::Edge<Weight> edge = {index, index + 1, wait_time};
        
        graph_.AddEdge(edge);
        
        // добавим данные во вспомогательные объекты
        stop_to_vertices_.emplace(stop.name, StopVertices{index, index + 1});
        edge_to_response_.emplace(edge, WaitResponse(stop.name, wait_time));
        
        index += 2;
    }
}

void TransportRouter::InitGraphBusEdges() {
    for (const catalogue::Bus& bus : catalogue_.GetBusesData()) {
        double travel_time = 0.0;
        for (auto from = bus.route.begin(), to = from + 1; to != bus.route.end(); ++from, ++to) {
            travel_time += catalogue_.GetDistanceBetweenStops(*from, *to) / settings_.velocity;
            
            
            
            graph::Edge<Weight> edge = {index, index + 1, wait_time};
            graph_.AddEdge(edge);
        }
    }
}

} // namespace router
