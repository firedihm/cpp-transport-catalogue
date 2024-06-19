#pragma once

#include "router.h"
#include "transport_catalogue.h"

#include <variant>

namespace router {

struct RoutingSettings {
    int wait_time = 0;
    double velocity = 0.0;
};

// тип элементов поля "items" ответа на запрос "Route"
struct WaitResponse {
    WaitResponse(std::string_view stop, double time) : stop(stop), time(time) {}
    
    const std::string type{"Wait"};
    std::string_view stop;
    double time;
};
struct BusResponse {
    BusResponse(std::string_view bus, int span, double time) : bus(bus), span(span), time(time) {}
    
    const std::string type{"Bus"};
    std::string_view bus;
    int span;
    double time;
};
using ResponseItem = std::variant<std::nullptr_t, WaitResponse, BusResponse>;

class TransportRouter {
public:
    using Weight = double;
    using RouteResponse = std::optional<std::pair<Weight, std::vector<ResponseItem>>>;
    
    TransportRouter(RoutingSettings&& settings, const catalogue::TransportCatalogue& catalogue)
        : settings_(std::move(settings)), catalogue_(catalogue)
        , graph_(catalogue_.GetStopsData().size() * 2), router_(graph_) {
        
        // вершины графа это остановки, рёбра – время ожидания на остановке или движения в автобусе
        InitGraphStopEdges();
        InitGraphBusEdges();
    }
    TransportRouter(const TransportRouter&) = delete;
    TransportRouter(TransportRouter&&) = delete;
    
    TransportRouter& operator=(const TransportRouter&) = delete;
    TransportRouter& operator=(TransportRouter&&) = delete;
    
    RouteResponse BuildRoute(std::string_view from, std::string_view to) const;
    
private:
    void InitGraphStopEdges();
    void InitGraphBusEdges();
    
    // граф храним отдельно, так как маршрутизатор ссылается на него
    RoutingSettings settings_;
    const catalogue::TransportCatalogue& catalogue_;
    graph::DirectedWeightedGraph<Weight> graph_;
    graph::Router<Weight> router_;
    
    // вспомогательные объекты для быстрого построения маршрутов после инициализации
    struct StopVertices { graph::VertexId begin, end; };
    
    struct EdgeHasher {
        size_t operator()(const graph::Edge<Weight>& edge) const {
            return hash(edge.from) + 31 * hash(edge.to) + 31 * 31 * hash_W(edge.weight);
        }
        
        std::hash<size_t> hash;
        std::hash<Weight> hash_W;
    };
    
    std::unordered_map<std::string_view, StopVertices> stop_to_vertices_;
    std::unordered_map<graph::Edge<Weight>, ResponseItem, EdgeHasher> edge_to_response_;
};

} // namespace router
