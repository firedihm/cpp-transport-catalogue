#include "json_builder.h"
#include "json_reader.h"

#include <sstream>

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

const Document JsonReader::ProcessStatRequests() {
    const Array& requests = input_.GetRoot().AsMap().at("stat_requests"s).AsArray();
    
    /*
    Array response;
    response.reserve(requests.size());
    for (const Node& request : requests) {
        std::string_view type = request.AsMap().at("type"s).AsString();
        if (type == "Bus"sv) {
            response.push_back(MakeBusResponse(request.AsMap()));
        } else if (type == "Stop"sv) {
            response.push_back(MakeStopResponse(request.AsMap()));
        } else if (type == "Map"sv) {
            response.push_back(MakeMapResponse(request.AsMap()));
        }
    }
    return Document(Node(response));
    */
    // нельзя объявить ctx как Builder::ArrayContext потому что он приватный, но как auto можно -- гениально
    Builder builder = json::Builder();
    auto /* Builder::ArrayContext */ ctx = builder.StartArray();
    for (const Node& request : requests) {
        std::string_view type = request.AsMap().at("type"s).AsString();
        if (type == "Bus"sv) {
            ctx.Value(MakeBusResponse(request.AsMap()));
        } else if (type == "Stop"sv) {
            ctx.Value(MakeStopResponse(request.AsMap()));
        } else if (type == "Map"sv) {
            ctx.Value(MakeMapResponse(request.AsMap()));
        }
    }
    return Document(ctx.EndArray().Build());
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

Dict JsonReader::MakeMapResponse(const Dict& request) {
    std::ostringstream oss;
    render::MapRenderer i(ParseRenderDetails(input_.GetRoot().AsMap().at("render_settings"s).AsMap()), catalogue_, oss);
    
    Dict response;
    response["request_id"s] = request.at("id"s).AsInt();
    response["map"s] = std::move(*oss.rdbuf()).str();
    return response;
}

void JsonReader::PrintStats(int step, int indent) {
    json::Print(ProcessStatRequests(), output_, step, indent);
}

void JsonReader::RenderMap() {
    render::MapRenderer i(ParseRenderDetails(input_.GetRoot().AsMap().at("render_settings"s).AsMap()), catalogue_, output_);
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

render::RenderDetails JsonReader::ParseRenderDetails(const Dict& settings) {
    render::RenderDetails details;
    
    details.width = settings.at("width"s).AsDouble();
    details.height = settings.at("height"s).AsDouble();
    details.padding = settings.at("padding"s).AsDouble();
    
    details.line_width = settings.at("line_width"s).AsDouble();
    details.stop_radius = settings.at("stop_radius"s).AsDouble();
    
    details.bus_label_font_size = settings.at("bus_label_font_size"s).AsInt();
    details.bus_label_offset = svg::Point(settings.at("bus_label_offset"s).AsArray()[0].AsDouble(),
                                          settings.at("bus_label_offset"s).AsArray()[1].AsDouble());
    
    details.stop_label_font_size = settings.at("stop_label_font_size"s).AsInt();
    details.stop_label_offset = svg::Point(settings.at("stop_label_offset"s).AsArray()[0].AsDouble(),
                                           settings.at("stop_label_offset"s).AsArray()[1].AsDouble());
    
    details.underlayer_color = ParseColor(settings.at("underlayer_color"s));
    details.underlayer_width = settings.at("underlayer_width"s).AsDouble();
    
    const Array& palette = settings.at("color_palette"s).AsArray();
    std::vector<svg::Color> colors;
    colors.reserve(palette.size());
    for (const Node& color : palette) {
        colors.push_back(ParseColor(color));
    }
    details.colors = std::move(colors);
    
    return details;
}

} // namespace json
