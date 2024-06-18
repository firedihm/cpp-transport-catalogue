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
        
        struct TempShortestTime { const Stop* from, to; int span; double time; };
        std::vector<TempShortestTime> shortest_times;
        shortest_times.reserve(bus.route.size());
        
        for (auto from = bus.route.begin(), to = from + 1; to != bus.route.end(); ++from, ++to) {
            travel_time += catalogue_.GetDistanceBetweenStops(*from, *to) / settings_.velocity;
            
            /*
             * Если автобус проезжает между некоторыми остановками несколько раз, 
             * то храним наименьшее время пути на этом отрезке
             */
            if (auto it = std::find_if(shortest_times.begin(), shortest_times.end(),
                                       [from, to](const TempShortestTime& elem) {
                                           return *from == elem.from && *to == elem.to;
                                       }); it == shortest_times.end()) {
                shortest_times.emplace_back(*from, *to, std::distance(from, to), travel_time);
            }
            
            for (const TempShortestTime& shortest_time : shortest_times) {
                graph::VertexId from_i = stop_to_vertices_[shortest_time.from->name];
                graph::VertexId to_i = stop_to_vertices_[shortest_time.to->name];
                graph::Edge<Weight> edge = {from_i, to_i, shortest_time.time};
                
                graph_.AddEdge(edge);
                
                edge_to_response_.emplace(edge, BusResponse(bus.name, shortest_time.span, shortest_time.time));
            }
        }
    }
}

std::pair<Weight, std::vector<ResponseItem>> TransportRouter::BuildRoute(std::string_view from,
                                                                            std::string_view to) const {
    std::optional<graph::Router<Weight>::RouteInfo> result();
}

} // namespace router
