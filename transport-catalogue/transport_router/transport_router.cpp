#include "transport_router.h"

namespace router {

void TransportRouter::InitGraphStopEdges(const std::deque<catalogue::Stop>& stops) {
    stop_to_vertices_.reserve(stops.size());
    edge_to_response_.reserve(stops.size());
    
    Weight wait_time = static_cast<Weight>(settings_.wait_time);
    for (graph::VertexId index = 0; const catalogue::Stop& stop : stops) {
        graph::Edge<Weight> edge = {index, index + 1, wait_time};
        
        graph_.AddEdge(edge);
        
        // добавим данные во вспомогательные объекты
        stop_to_vertices_.emplace(stop.name, StopVertices{index, index + 1});
        edge_to_response_.emplace(edge, WaitResponse(stop.name, wait_time));
        
        index += 2;
    }
}

void TransportRouter::InitGraphBusEdges(const std::deque<catalogue::Bus>& buses) {
    
}

} // namespace router
