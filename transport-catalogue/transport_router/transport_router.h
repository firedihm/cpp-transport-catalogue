#pragma once

#include "domain.h"
#include "router.h"

#include <deque>
#include <variant>

namespace router {

struct RoutingSettings {
    int wait_time = 0;
    double velocity = 0.0;
};

struct WaitResponse {
    WaitResponse(std::string_view stop, double time) : stop(stop), time(time) {}
    
    const std::string type{"Wait"};
    const std::string_view stop;
    const double time;
};
struct BusResponse {
    BusResponse(std::string_view bus, int span_count, double time) : bus(bus), span_count(span_count), time(time) {}
    
    const std::string type{"Bus"};
    const std::string_view bus;
    const int span_count;
    const double time;
};
using ResponseItem = std::variant<WaitResponse, BusResponse>;

class TransportRouter {
public:
    TransportRouter(RoutingSettings&& settings, const std::deque<catalogue::Stop>& stops,
                                                const std::deque<catalogue::Bus>& buses)
        : settings_(std::move(settings)), graph_(stops.size() * 2), router_(graph_) {
        
        InitGraphStopEdges(stops);
        InitGraphBusEdges(buses);
    }
    TransportRouter(const TransportRouter&) = delete;
    TransportRouter(TransportRouter&&) = delete;
    
    TransportRouter& operator=(const TransportRouter&) = delete;
    TransportRouter& operator=(TransportRouter&&) = delete;
    
private:
    using Weight = double;
    
    struct StopVertices { graph::VertexId from, to; };
    
    struct EdgeHasher {
        size_t operator()(const graph::Edge<Weight>& edge) const {
            return hash(edge.from) + 31 * hash(edge.to) + 31 * 31 * hash_W(edge.weight);
        }
        
        std::hash<size_t> hash;
        std::hash<Weight> hash_W;
    };
    
    void InitGraphStopEdges(const std::deque<catalogue::Stop>& stops);
    void InitGraphBusEdges(const std::deque<catalogue::Bus>& buses);
    
    // граф храним отдельно, так как маршрутизатор ссылается на него
    RoutingSettings settings_;
    graph::DirectedWeightedGraph<Weight> graph_;
    graph::Router<Weight> router_;
    
    // вспомогательные объекты для быстрого построения маршрутов после инициализации
    std::unordered_map<std::string_view, StopVertices> stop_to_vertices_;
    std::unordered_map<graph::Edge<Weight>, ResponseItem, EdgeHasher> edge_to_response_;
};

} // namespace router
