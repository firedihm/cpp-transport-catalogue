#include "transport_catalogue.h"

#include <algorithm>

namespace catalogue {

const Stop* TransportCatalogue::GetStop(std::string_view key) const {
    auto it = stops_view_.find(key);
    return it != stops_view_.end() ? it->second : nullptr;
}

const Bus* TransportCatalogue::GetBus(std::string_view key) const {
    auto it = buses_view_.find(key);
    return it != buses_view_.end() ? it->second : nullptr;
}

const std::set<std::string_view>* TransportCatalogue::GetBusesForStop(const Stop* stop) const {
    auto it = stop_to_buses_.find(stop);
    return it != stop_to_buses_.end() ? &(it->second) : nullptr;
}

int TransportCatalogue::GetDistanceBetweenStops(const Stop* from, const Stop* to) const {
    auto it = distances_.find(std::pair(from, to));
    return it != distances_.end() ? it->second
                                  : (it = distances_.find(std::pair(to, from))) != distances_.end() ? it->second : 0;
}

void TransportCatalogue::AddStop(const std::string& id, geo::Coordinates&& coords) {
    const Stop& ref = stops_.emplace_back(id, std::move(coords));
    stops_view_.emplace(ref.name, &ref);
    stop_to_buses_[&ref];
}

void TransportCatalogue::AddDistance(const std::string& id, std::vector<std::pair<std::string_view, int>>&& distances) {
    for (const auto& [stop, distance] : distances) {
        distances_.emplace(std::pair(GetStop(id), GetStop(stop)), distance);
    }
}

void TransportCatalogue::AddBus(const std::string& id, std::vector<std::string_view>&& route, bool is_ring) {
    // маршрут строится как последовательность указателей на соответствующие названиям из route остановки.
    std::vector<const Stop*> stop_ptrs;
    stop_ptrs.reserve(is_ring ? route.size() : 2 * route.size() - 1);
    for (std::string_view stop : route) {
        const Stop* stop_ptr = GetStop(stop);
        stop_ptrs.push_back(stop_ptr);
        
        // обновляем здесь, чтобы при масштабировании не учитывать остановки без автобусов; сначала min...
        min_max_coords_.first.lat = std::min(min_max_coords_.first.lat, stop_ptr->coords.lat);
        min_max_coords_.first.lng = std::min(min_max_coords_.first.lng, stop_ptr->coords.lng);
        // ...потом max
        min_max_coords_.second.lat = std::max(min_max_coords_.second.lat, stop_ptr->coords.lat);
        min_max_coords_.second.lng = std::max(min_max_coords_.second.lng, stop_ptr->coords.lng);
    }
    // отзеркалим некольцевые маршруты
    if (!is_ring) {
        for (auto it = stop_ptrs.rbegin() + 1; it != stop_ptrs.rend(); ++it) {
            stop_ptrs.push_back(*it);
        }
    }
    
    const Bus& ref = buses_.emplace_back(id, std::move(stop_ptrs), is_ring ? RouteType::RING : RouteType::PENDULUM);
    buses_view_.emplace(ref.name, &ref);
    
    for (const Stop* stop : ref.route) {
        stop_to_buses_.at(stop).insert(ref.name);
    }
}

int TransportCatalogue::CountUniqueStops(const Bus* bus) {
    std::vector<const Stop*> sorted(bus->route);
    std::sort(sorted.begin(), sorted.end());
    
    // считать будем только количество разных по значению соседних элементов
    int unique = 0;
    for (uint i = 0; i < sorted.size(); ++i) {
        // пока соседние элементы равны пропускаем итерацию
        while (i < sorted.size() - 1 && sorted[i] == sorted[i + 1]) {
            ++i;
        }
        ++unique;
    }
    return unique;
}

double TransportCatalogue::CalculateRouteGeoLength(const Bus* bus) {
    double length = 0.0;
    for (auto curr = bus->route.begin(), next = curr + 1; next != bus->route.end(); ++curr, ++next) {
        length += geo::ComputeDistance((*curr)->coords, (*next)->coords);
    }
    return length;
}

int TransportCatalogue::CalculateRouteLength(const Bus* bus) const {
    int length = 0;
    for (auto curr = bus->route.begin(), next = curr + 1; next != bus->route.end(); ++curr, ++next) {
        length += GetDistanceBetweenStops(*curr, *next);
    }
    return length;
}

} // namespace catalogue
