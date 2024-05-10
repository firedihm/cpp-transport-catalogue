#pragma once

#include "domain.h"

#include <cfloat>
#include <deque>
#include <set>
#include <unordered_map>

namespace catalogue {

class TransportCatalogue {
public:
    struct MinMaxCoords { geo::Coordinates min, max; };
    
    TransportCatalogue() = default;
    TransportCatalogue(const TransportCatalogue&) = delete;
    TransportCatalogue(TransportCatalogue&&) = delete;
    
    TransportCatalogue& operator=(const TransportCatalogue&) = delete;
    TransportCatalogue& operator=(TransportCatalogue&&) = delete;
    
    const Stop* GetStop(std::string_view key) const;
    const Bus* GetBus(std::string_view key) const;
    const std::set<std::string_view>* GetBusesForStop(const Stop* stop) const;
    int GetDistanceBetweenStops(const Stop* from, const Stop* to) const;
    
    inline const std::deque<Stop>& GetStopsData() const { return stops_; };
    inline const std::deque<Bus>& GetBusesData() const { return buses_; };
    inline const MinMaxCoords& GetMinMaxCoords() const { return min_max_coords_; };
    
    void AddStop(const std::string& id, geo::Coordinates&& coords);
    void AddDistance(const std::string& source, const std::string_view destination, int distance);
    void AddBus(const std::string& id, std::vector<std::string_view>&& route, bool is_ring);
    
    static int CountUniqueStops(const Bus* bus);
    static double CalculateRouteGeoLength(const Bus* bus);
    int CalculateRouteLength(const Bus* bus) const;
    
private:
    struct StopPtrsHasher {
        size_t operator()(const std::pair<const Stop*, const Stop*>& pair) const {
            return hash(pair.first) + 31 * hash(pair.second);
        }
        
        std::hash<const void*> hash;
    };
    
    std::deque<Stop> stops_;
    std::unordered_map<std::string_view, const Stop*> stops_view_;
    
    std::deque<Bus> buses_;
    std::unordered_map<std::string_view, const Bus*> buses_view_;
    
    std::unordered_map<const Stop*, std::set<std::string_view>> stop_to_buses_;
    std::unordered_map<std::pair<const Stop*, const Stop*>, int, StopPtrsHasher> distances_;
    
    // для рендера: при обновлении справочника будем запоминать маргинальные координаты <min, max>
    MinMaxCoords min_max_coords_{{DBL_MAX, DBL_MAX}, {-DBL_MAX, -DBL_MAX}};
};

} // namespace catalogue
