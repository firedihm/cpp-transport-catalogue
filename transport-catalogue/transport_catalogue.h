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
    std::vector<Stop*> route;
    
    uint CountUniqueStops() const;
};

class TransportCatalogue {
public:
    TransportCatalogue() = default;
    TransportCatalogue(const TransportCatalogue&) = delete;
    TransportCatalogue(TransportCatalogue&&) = delete;
    
    TransportCatalogue& operator=(const TransportCatalogue&) = delete;
    TransportCatalogue& operator=(TransportCatalogue&&) = delete;
    
    Stop* GetStop(std::string_view key) const;
    Bus* GetBus(std::string_view key) const;
    const std::set<std::string_view>& GetBusesForStop(Stop* stop) const;
    
    void AddStop(const std::string& id, geo::Coordinates&& coords);
    void AddBus(const std::string& id, std::vector<std::string_view>&& route);
    
private:
    std::deque<Stop> stops_;
    std::unordered_map<std::string_view, Stop*> stops_view_;
    
    std::deque<Bus> buses_;
    std::unordered_map<std::string_view, Bus*> buses_view_;
    
    std::unordered_map<Stop*, std::set<std::string_view>> stop_to_buses_;
};

} // namespace catalogue
