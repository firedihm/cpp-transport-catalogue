#pragma once

#include "router.h"
#include "transport_catalogue.h"

#include <memory>
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
using ResponseItem = std::variant<WaitResponse, BusResponse>;

class TransportRouter {
public:
    using Weight = double;
    struct RouteResponse { Weight weight; std::vector<ResponseItem> response_items; };
    
    TransportRouter(RoutingSettings&& settings, const catalogue::TransportCatalogue& catalogue)
        : settings_(std::move(settings)), catalogue_(catalogue)
        , graph_(catalogue_.GetStopsData().size() * 2) {
        
        // вершины графа это остановки, рёбра – время ожидания на остановке или движения в автобусе
        InitGraphWaitEdges();
        InitGraphBusEdges();
        
        router_ = std::make_unique<graph::Router<Weight>>(graph_);
    }
    TransportRouter(const TransportRouter&) = delete;
    TransportRouter(TransportRouter&&) = delete;
    
    TransportRouter& operator=(const TransportRouter&) = delete;
    TransportRouter& operator=(TransportRouter&&) = delete;
    
    std::optional<RouteResponse> BuildRoute(std::string_view from, std::string_view to) const;
    
private:
    void InitGraphWaitEdges();
    void InitGraphBusEdges();
    
    RoutingSettings settings_;
    const catalogue::TransportCatalogue& catalogue_;
    
    // маршрутизатор нужно создавать после графа, поэтому объявим его как std::unique_ptr
    graph::DirectedWeightedGraph<Weight> graph_;
    std::unique_ptr<graph::Router<Weight>> router_;
    
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
