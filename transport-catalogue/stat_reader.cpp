#include "stat_reader.h"

#include <iostream>
#include <stdexcept>

namespace catalogue::stat {

void ParseAndPrintStat(const TransportCatalogue& catalogue, std::string_view request, std::ostream& output) {
    using namespace std::string_literals;
    
    size_t delim = request.find(' ');
    std::string_view command = request.substr(request.find_first_not_of(' '), delim);
    request = request.substr(request.find_first_not_of(' ', std::move(++delim)), request.find_last_not_of(' ') + 1);
    
    if (command == "Bus"s) {
        try {
            const Bus* bus = catalogue.GetBus(request);
            
            int route_length = catalogue.CalculateRouteLength(bus);
            
            output << "Bus "s << request << ": "s
                   << bus->route.size() << " stops on route, "s
                   << catalogue.CountUniqueStops(bus) << " unique stops, "s
                   << route_length << " route length, "s
                   << route_length / catalogue.CalculateRouteGeoLength(bus) << " curvature\n"s;
        } catch (std::out_of_range& e) {
            output << "Bus "s << request << ": not found\n"s;
        }
    } else if (command == "Stop"s) {
        try {
            const std::set<std::string_view>& buses_for_stop = catalogue.GetBusesForStop(catalogue.GetStop(request));
            
            if (!buses_for_stop.empty()) {
                output << "Stop "s << request << ": buses"s;
                for (std::string_view bus : buses_for_stop) {
                    output << ' ' << bus;
                }
                output << '\n';
            } else {
                output << "Stop "s << request << ": no buses\n"s;
            }
        } catch (std::out_of_range& e) {
            output << "Stop "s << request << ": not found\n"s;
        }
    }
}

} // namespace catalogue::stats
