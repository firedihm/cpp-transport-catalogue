#include "geo.h"
#include "transport_catalogue.h"

#include <algorithm>
#include <stdexcept>

namespace catalogue {

const Stop* TransportCatalogue::GetStop(std::string_view key) const {
    try {
        return stops_view_.at(key);
    } catch (std::out_of_range& e) {
        throw e;
    }
}

const Bus* TransportCatalogue::GetBus(std::string_view key) const {
    try {
        return buses_view_.at(key);
    } catch (std::out_of_range& e) {
        throw e;
    }
}

const std::set<std::string_view>& TransportCatalogue::GetBusesForStop(const Stop* stop) const {
    try {
        return stop_to_buses_.at(stop);
    } catch (std::out_of_range& e) {
        throw e;
    }
}

int TransportCatalogue::GetDistanceBetweenStops(const Stop* from, const Stop* to) const {
    try {
        return distances_.at(std::pair(from, to));
    } catch (std::out_of_range& e) {
        try {
            return distances_.at(std::pair(to, from));
        } catch (std::out_of_range& e) {
            return 0;
        }
    }
}

void TransportCatalogue::AddStop(const std::string& id, geo::Coordinates&& coords) {
    Stop& ref = stops_.emplace_back(id, std::move(coords));
    stops_view_.emplace(ref.name, &ref);
    stop_to_buses_[&ref];
}

void TransportCatalogue::AddDistance(const std::string& id, std::vector<std::pair<std::string_view, int>>&& distances) {
    for (const auto& [stop, distance] : distances) {
        distances_.emplace(std::pair(GetStop(id), GetStop(stop)), distance);
    }
}

void TransportCatalogue::AddBus(const std::string& id, std::vector<std::string_view>&& route) {
    std::vector<const Stop*> stop_ptrs;
    stop_ptrs.reserve(route.size());
    for (std::string_view stop : route) {
        stop_ptrs.push_back(GetStop(stop));
    }
    
    const Bus& ref = buses_.emplace_back(id, std::move(stop_ptrs));
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
    for (int i = 0; i < sorted.size(); ++i) {
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
