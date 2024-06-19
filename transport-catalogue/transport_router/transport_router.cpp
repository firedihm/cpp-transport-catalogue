#include "transport_router.h"

using namespace catalogue;
#include <iostream>
namespace router {

void TransportRouter::InitGraphWaitEdges() {
    stop_to_vertices_.reserve(catalogue_.GetStopsData().size());
    Weight wait_time = static_cast<Weight>(settings_.wait_time);
    
    for (graph::VertexId index = 0; const Stop& stop : catalogue_.GetStopsData()) {
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
        /*
         * Если автобус проезжает между некоторыми остановками несколько раз, 
         * то храним наименьшее время пути на этом отрезке.
         */
        struct Record { double time; int span; };
        std::unordered_map<std::pair<const Stop*, const Stop*>, Record, TransportCatalogue::StopPtrsHasher> span_to_time;
        
        // оценим все возможные отрезки на маршруте автобуса
        for (auto from = bus.route.begin(); from != bus.route.end() - 1; ++from) {
            Weight travel_time = 0.0;
            
            for (auto current = from, to = from + 1; to != bus.route.end(); ++current, ++to) {
                travel_time += catalogue_.GetDistanceBetweenStops(*current, *to) / settings_.velocity;
                
                std::pair<const Stop*, const Stop*> key{*from, *to};
                Record value{travel_time, static_cast<int>(std::distance(from, to))};
                if (auto it = span_to_time.find(key); it == span_to_time.end()) {
                    span_to_time[key] = value;
                } else if (travel_time < it->second.time) {
                    it->second = value;
                }
            }
        }
        
        for (const auto& [span, record] : span_to_time) {
            graph::Edge<Weight> edge{stop_to_vertices_[span.first->name].end,
                                     stop_to_vertices_[span.second->name].begin,
                                     record.time};
            
            graph_.AddEdge(edge);
            
            // добавим данные во вспомогательные объекты
            edge_to_response_.emplace(edge, BusResponse(bus.name, record.span, record.time));
        }
    }
}

std::optional<TransportRouter::RouteResponse> TransportRouter::BuildRoute(std::string_view from,
                                                                          std::string_view to) const {
    std::optional<RouteResponse> result(std::nullopt);
    if (auto route = router_->BuildRoute(stop_to_vertices_.at(from).begin, stop_to_vertices_.at(to).begin)) {
        std::vector<ResponseItem> response_items;
        for (graph::EdgeId edge_id : route->edges) {
            response_items.push_back(edge_to_response_.at(graph_.GetEdge(edge_id)));
        }
        
        result.emplace(route->weight, std::move(response_items));
    }
    return result;
}

} // namespace router
