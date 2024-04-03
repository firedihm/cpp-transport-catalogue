#include "stat_reader.h"

#include <iostream>
#include <stdexcept>

namespace catalogue::stat {

void ParseAndPrintStat(const TransportCatalogue& catalogue, std::string_view request, std::ostream& output) {
    using namespace std::literals;
    
    size_t delim = request.find_first_of(' ');
    std::string_view command = request.substr(0, delim);
    request = request.substr(std::move(++delim));
    if (command == "Bus"s) {
        try {
            Bus* bus_info = catalogue.GetBus(request);
            
            double length = 0.0;
            for (auto curr = bus_info->route.begin(), next = curr + 1; next != bus_info->route.end(); ++curr, ++next) {
                length += geo::ComputeDistance((*curr)->coords, (*next)->coords);
            }
            
            output << "Bus "s << request << ": "s
                   << bus_info->route.size() << " stops on route, "s
                   << bus_info->CountUniqueStops() << " unique stops, "s
                   << std::move(length) << " route length\n"s;
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
