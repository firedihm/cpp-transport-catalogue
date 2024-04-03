#include "geo.h"
#include "transport_catalogue.h"

#include <algorithm>
#include <stdexcept>

namespace catalogue {

uint Bus::CountUniqueStops() const {
    /*
     * https://www.geeksforgeeks.org/count-distinct-elements-in-an-array/
     */
    std::vector<Stop*> sorted(route);
    std::sort(sorted.begin(), sorted.end());
    
    // считать будем только количество разных по значению соседних элементов
    uint unique = 0;
    for (uint i = 0; i < sorted.size(); ++i) {
        // пока соседние элементы равны пропускаем итерацию
        while (i < sorted.size() - 1 && sorted[i] == sorted[i + 1]) {
            ++i;
        }
        ++unique;
    }
    return unique;
}

Stop* TransportCatalogue::GetStop(std::string_view key) const {
    try {
        return stops_view_.at(key);
    } catch (std::out_of_range& e) {
        throw e;
    }
}

Bus* TransportCatalogue::GetBus(std::string_view key) const {
    try {
        return buses_view_.at(key);
    } catch (std::out_of_range& e) {
        throw e;
    }
}

const std::set<std::string_view>& TransportCatalogue::GetBusesForStop(Stop* stop) const {
    return stop_to_buses_.at(stop);
}

void TransportCatalogue::AddStop(const std::string& id, geo::Coordinates&& coords) {
    Stop& ref = stops_.emplace_back(id, std::move(coords));
    stops_view_.emplace(ref.name, &ref);
    stop_to_buses_[&ref];
}

void TransportCatalogue::AddBus(const std::string& id, std::vector<std::string_view>&& route) {
    std::vector<Stop*> stop_ptrs;
    stop_ptrs.reserve(route.size());
    for (std::string_view stop : route) {
        stop_ptrs.push_back(GetStop(stop));
    }
    
    Bus& ref = buses_.emplace_back(id, std::move(stop_ptrs));
    buses_view_.emplace(ref.name, &ref);
    
    for (Stop* stop : ref.route) {
        stop_to_buses_.at(stop).insert(ref.name);
    }
}

} // namespace catalogue
