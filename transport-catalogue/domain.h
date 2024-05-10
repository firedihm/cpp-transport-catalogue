#pragma once

#include "geo.h"

#include <string>
#include <vector>

namespace catalogue {

struct Stop {
    std::string name;
    geo::Coordinates coords;
};

enum class RouteType { RING, PENDULUM };

struct Bus {
    std::string name;
    std::vector<const Stop*> route;
    RouteType type;
};

} // namespace catalogue
