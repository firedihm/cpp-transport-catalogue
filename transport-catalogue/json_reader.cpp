#include "json_builder.h"
#include "json_reader.h"

#include <sstream>
#include <iostream>

using namespace std::literals;
using namespace catalogue;

namespace json {

void JsonReader::ProcessBaseRequests() {
    const Array& requests = input_.GetRoot().AsMap().at("base_requests"s).AsArray();
    
    // запомним отложенные запросы, чтобы не итерировать по всему массиву на каждом этапе
    std::vector<uint> distances, buses;
    distances.reserve(requests.size());
    buses.reserve(requests.size());
    
    // сначала инициализируем остановки...
    for (uint id = 0; id < requests.size(); ++id) {
        const Dict& request = requests[id].AsMap();
        std::string_view type = request.at("type"s).AsString();
        
        if (type == "Stop"sv) {
            catalogue_.AddStop(request.at("name"s).AsString(), ParseCoordinates(request));
            if (!request.at("road_distances"s).AsMap().empty()) {
                distances.push_back(id);
            }
        } else if (type == "Bus"sv) {
            buses.push_back(id);
        }
    }
    
    // ...затем расстояния между ними...
    for (uint id : distances) {
        const Dict& request = requests[id].AsMap();
        for (const auto& /* <std::pair<std::string_view, int>> */ [destination, distance] : ParseDistances(request)) {
            catalogue_.AddDistance(request.at("name"s).AsString(), destination, distance);
        }
    }
    
    // ...потом маршруты
    for (uint id : buses) {
        const Dict& request = requests[id].AsMap();
        catalogue_.AddBus(request.at("name"s).AsString(), ParseRoute(request), request.at("is_roundtrip"s).AsBool());
    }
}

void JsonReader::PrintStats(int step, int indent) {
    json::Print(ProcessStatRequests(), output_, step, indent);
}

void JsonReader::RenderMap(int step, int indent) {
    render::MapRenderer i(ParseRenderSettings(input_.GetRoot().AsMap().at("render_settings"s).AsMap()), catalogue_);
    i.RenderMap(output_, step, indent);
}

const Document JsonReader::ProcessStatRequests() {
    const Array& requests = input_.GetRoot().AsMap().at("stat_requests"s).AsArray();
    
    // вспомогательные объекты будут инициализироваться только если поступит соответствующий запрос
    MapRenderer renderer(nullptr);
    TransportRouter router(nullptr);
    
    Array response;
    response.reserve(requests.size());
    for (const Node& request : requests) {
        std::string_view type = request.AsMap().at("type"s).AsString();
        if (type == "Bus"sv) {
            response.push_back(MakeBusResponse(request.AsMap()));
        } else if (type == "Stop"sv) {
            response.push_back(MakeStopResponse(request.AsMap()));
        } else if (type == "Map"sv) {
            if (!renderer.get()) {
                const Dict& settings = input_.GetRoot().AsMap().at("render_settings"s).AsMap();
                renderer = std::make_unique<render::MapRenderer>(ParseRenderSettings(settings), catalogue_);
            }
            response.push_back(MakeMapResponse(request.AsMap(), renderer));
        } else if (type == "Route"sv) {
            if (!router.get()) {
                const Dict& settings = input_.GetRoot().AsMap().at("routing_settings"s).AsMap();
                router = std::make_unique<router::TransportRouter>(ParseRouteSettings(settings), catalogue_);
            }
            response.push_back(MakeRouteResponse(request.AsMap(), router));
        }
    }
    return Document(Node(response));
    
    /* нельзя объявить ctx как Builder::ArrayContext потому что он приватный, но как auto можно -- гениально
    Builder builder = json::Builder();
    auto ctx = builder.StartArray();
    for (const Node& request : requests) {
        std::string_view type = request.AsMap().at("type"s).AsString();
        if (type == "Bus"sv) {
            ctx.Value(MakeBusResponse(request.AsMap()));
        } else if (type == "Stop"sv) {
            ctx.Value(MakeStopResponse(request.AsMap()));
        } else if (type == "Map"sv) {
            if (!renderer.get()) {
                const Dict& settings = input_.GetRoot().AsMap().at("render_settings"s).AsMap();
                renderer = std::make_unique<render::MapRenderer>(ParseRenderSettings(settings), catalogue_);
            }
            ctx.Value(MakeMapResponse(request.AsMap(), renderer));
        } else if (type== "Route"sv) {
            if (!router.get()) {
                const Dict& settings = input_.GetRoot().AsMap().at("routing_settings"s).AsMap();
                router = std::make_unique<router::TransportRouter>(ParseRouteSettings(settings), catalogue_);
            }
            ctx.Value(MakeRouteResponse(request.AsMap(), router));
        }
    }
    return Document(ctx.EndArray().Build());
    */
}

Dict JsonReader::MakeBusResponse(const Dict& request) {
    Dict response;
    if (const Bus* bus = catalogue_.GetBus(request.at("name"s).AsString())) {
        double route_length = static_cast<double>(catalogue_.CalculateRouteLength(bus));
        response["request_id"s] = request.at("id"s).AsInt();
        response["stop_count"s] = static_cast<int>(bus->route.size());
        response["unique_stop_count"s] = catalogue_.CountUniqueStops(bus);
        response["route_length"s] = route_length;
        response["curvature"s] = route_length / catalogue_.CalculateRouteGeoLength(bus);
    } else {
        response["request_id"s] = request.at("id"s).AsInt();
        response["error_message"s] = "not found"s;
    }
    return response;
}

Dict JsonReader::MakeStopResponse(const Dict& request) {
    Dict response;
    if (const Stop* stop = catalogue_.GetStop(request.at("name"s).AsString())) {
        const std::set<std::string_view>* buses = catalogue_.GetBusesForStop(stop);
        
        //Array routes(buses->begin(), buses->end()); // не работает -- БРЕД
        Array routes;
        for (auto it = buses->begin(); it != buses->end(); ++it) {
            routes.emplace_back(std::string(*it));
        }
        response["request_id"s] = request.at("id"s).AsInt();
        response["buses"s] = std::move(routes);
    } else {
        response["request_id"s] = request.at("id"s).AsInt();
        response["error_message"s] = "not found"s;
    }
    return response;
}

Dict JsonReader::MakeMapResponse(const Dict& request, const MapRenderer& renderer) {
    std::ostringstream oss;
    renderer->RenderMap(oss, 0, 4);
    
    Dict response;
    response["request_id"s] = request.at("id"s).AsInt();
    response["map"s] = std::move(*oss.rdbuf()).str();
    return response;
}

Dict JsonReader::MakeRouteResponse(const Dict& request, const TransportRouter& router) {
    Dict response;
    if (std::optional<router::TransportRouter::RouteResponse> route_info =
            router->BuildRoute(request.at("from"s).AsString(), request.at("to"s).AsString())) {
        Array items;
        for (auto it = route_info->response_items.begin(); it != route_info->response_items.end(); ++it) {
            std::visit([&items](const auto& item) {
                Dict result;
                result["type"s] = item.type;
                if constexpr (std::is_same_v<std::decay_t<decltype(item)>, router::WaitResponse>) {
                    result["stop_name"s] = std::string(item.stop);
                } else if constexpr (std::is_same_v<std::decay_t<decltype(item)>, router::BusResponse>) {
                    result["bus"s] = std::string(item.bus);
                    result["span_count"s] = item.span;
                }
                result["time"s] = item.time;
                
                items.push_back(std::move(result));
            }, *it);
        }
        
        response["request_id"s] = request.at("id"s).AsInt();
        response["total_time"s] = route_info->weight;
        response["items"s] = std::move(items);
    } else {
        response["request_id"s] = request.at("id"s).AsInt();
        response["error_message"s] = "not found"s;
    }
    return response;
}

geo::Coordinates JsonReader::ParseCoordinates(const Dict& request) {
    return { request.at("latitude"s).AsDouble(), request.at("longitude"s).AsDouble() };
}

std::vector<std::pair<std::string_view, int>> JsonReader::ParseDistances(const Dict& request) {
    const Dict& data = request.at("road_distances"s).AsMap();
    
    std::vector<std::pair<std::string_view, int>> result;
    result.reserve(data.size());
    for (const auto& /* std::string, Node */ [destination, distance] : data) {
        result.emplace_back(destination, distance.AsInt());
    }
    return result;
}

std::vector<std::string_view> JsonReader::ParseRoute(const Dict& request) {
    const Array& data = request.at("stops"s).AsArray();
    
    std::vector<std::string_view> result;
    result.reserve(data.size());
    for (const Node& entry : data) {
        result.emplace_back(entry.AsString());
    }
    return result;
}

svg::Color JsonReader::ParseColor(const Node& node) {
    if (node.IsString()) {
        return node.AsString();
    }
    
    const Array& color = node.AsArray();
    if (color.size() == 3) {
        return svg::Rgb(color[0].AsInt(), color[1].AsInt(), color[2].AsInt());
    } else {
        return svg::Rgba(color[0].AsInt(), color[1].AsInt(), color[2].AsInt(), color[3].AsDouble());
    }
}

render::RenderSettings JsonReader::ParseRenderSettings(const Dict& settings) {
    render::RenderSettings result;
    
    result.width = settings.at("width"s).AsDouble();
    result.height = settings.at("height"s).AsDouble();
    result.padding = settings.at("padding"s).AsDouble();
    
    result.line_width = settings.at("line_width"s).AsDouble();
    result.stop_radius = settings.at("stop_radius"s).AsDouble();
    
    result.bus_label_font_size = settings.at("bus_label_font_size"s).AsInt();
    result.bus_label_offset = svg::Point(settings.at("bus_label_offset"s).AsArray()[0].AsDouble(),
                                          settings.at("bus_label_offset"s).AsArray()[1].AsDouble());
    
    result.stop_label_font_size = settings.at("stop_label_font_size"s).AsInt();
    result.stop_label_offset = svg::Point(settings.at("stop_label_offset"s).AsArray()[0].AsDouble(),
                                           settings.at("stop_label_offset"s).AsArray()[1].AsDouble());
    
    result.underlayer_color = ParseColor(settings.at("underlayer_color"s));
    result.underlayer_width = settings.at("underlayer_width"s).AsDouble();
    
    const Array& palette = settings.at("color_palette"s).AsArray();
    std::vector<svg::Color> colors;
    colors.reserve(palette.size());
    for (const Node& color : palette) {
        colors.push_back(ParseColor(color));
    }
    result.colors = std::move(colors);
    
    return result;
}

router::RoutingSettings JsonReader::ParseRouteSettings(const Dict& settings) {
    const double KMH_TO_MMIN = 1000.0 / 60.0;
    
    return { settings.at("bus_wait_time"s).AsInt(), settings.at("bus_velocity"s).AsDouble() * KMH_TO_MMIN };
}

} // namespace json
