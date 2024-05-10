#pragma once

#include "json.h"
#include "transport_catalogue.h"

namespace json {

void ParseAndPrintRequests(catalogue::TransportCatalogue& catalogue, const Document& input, std::ostream& output, int step = 4, int indent = 0);
void ParseAndRenderMap(catalogue::TransportCatalogue& catalogue, const Document& input, std::ostream& output, int step = 0, int indent = 4);

} // namespace json
