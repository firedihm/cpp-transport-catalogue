#pragma once

#include "transport_catalogue.h"

#include <iosfwd>
//#include <string_view>

namespace catalogue::stat {

void ParseAndPrintStat(const TransportCatalogue& catalogue, std::string_view request, std::ostream& output);

} // namespace catalogue::stats
