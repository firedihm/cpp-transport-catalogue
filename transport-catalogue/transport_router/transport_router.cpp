#include "transport_router.h"

using namespace catalogue;

namespace router {

void TransportRouter::InitGraphStopEdges() {
    const std::deque<Stop>& stops = catalogue_.GetStopsData();
    
    stop_to_vertices_.reserve(stops.size());
    edge_to_response_.reserve(stops.size());
    
    Weight wait_time = static_cast<Weight>(settings_.wait_time);
    for (graph::VertexId index = 0; const Stop& stop : stops) {
        graph::Edge<Weight> edge{index, index + 1, wait_time};
        
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
        
        /*
         * Если автобус проезжает между некоторыми остановками несколько раз, 
         * то храним наименьшее время пути на этом отрезке, т.е. оставляем первую 
         * запись для отрезка, потому что travel_time растёт с каждой итерацией. 
         * В структуре будем хранить уже обработанные отрезки.
         */
        struct ProcessedSpan { const Stop* from, * to; };
        std::vector<ProcessedSpan> processed_spans;
        processed_spans.reserve(bus.route.size());
        
        for (auto from = bus.route.begin(), to = from + 1; to != bus.route.end(); ++from, ++to) {
            travel_time += catalogue_.GetDistanceBetweenStops(*from, *to) / settings_.velocity;
            
            // если текущий отрезок ещё не повторялся...
            if (auto it = std::find_if(processed_spans.begin(), processed_spans.end(),
                                       [from, to](const ProcessedSpan& span) {
                                           return (*from == span.from) && (*to == span.to);
                                       }); it == processed_spans.end()) {
                processed_spans.emplace_back(*from, *to);
                
                graph::Edge<Weight> edge{stop_to_vertices_[(*from)->name].end,
                                         stop_to_vertices_[(*to)->name].begin,
                                         travel_time};
                
                graph_.AddEdge(edge);
                
                // добавим данные во вспомогательные объекты
                edge_to_response_.emplace(edge, BusResponse(bus.name, std::distance(from, to), travel_time));
            }
        }
    }
}

TransportRouter::RouteResponse TransportRouter::BuildRoute(std::string_view from, std::string_view to) const {
    RouteResponse result;
    if (auto route = router_.BuildRoute(stop_to_vertices_.at(from).begin, stop_to_vertices_.at(to).begin)) {
        std::vector<ResponseItem> responses;
        for (graph::EdgeId edge_id : route->edges) {
            responses.push_back(edge_to_response_.at(graph_.GetEdge(edge_id)));
        }
        
        result = std::pair(route->weight, std::move(responses));
    }
    return result;
}

} // namespace router
