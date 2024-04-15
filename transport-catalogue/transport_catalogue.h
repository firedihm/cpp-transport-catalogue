#pragma once

#include "geo.h"

#include <deque>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace catalogue {

struct Stop {
    std::string name;
    geo::Coordinates coords;
};

struct Bus {
    std::string name;
    std::vector<const Stop*> route;
};

class TransportCatalogue {
public:
    TransportCatalogue() = default;
    TransportCatalogue(const TransportCatalogue&) = delete;
    TransportCatalogue(TransportCatalogue&&) = delete;
    
    TransportCatalogue& operator=(const TransportCatalogue&) = delete;
    TransportCatalogue& operator=(TransportCatalogue&&) = delete;
    
    const Stop* GetStop(std::string_view key) const;
    const Bus* GetBus(std::string_view key) const;
    const std::set<std::string_view>& GetBusesForStop(const Stop* stop) const;
    int GetDistanceBetweenStops(const Stop* from, const Stop* to) const;
    
    void AddStop(const std::string& id, geo::Coordinates&& coords);
    void AddDistance(const std::string& id, std::vector<std::pair<std::string_view, int>>&& distances);
    void AddBus(const std::string& id, std::vector<std::string_view>&& route);
    
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
};

} // namespace catalogue
