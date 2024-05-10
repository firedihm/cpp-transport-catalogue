#include "geo.h"

#define _USE_MATH_DEFINES
#include <cmath>

namespace catalogue::geo {

bool Coordinates::operator==(const Coordinates& rhs) const {
    return lat == rhs.lat && lng == rhs.lng;
}

bool Coordinates::operator!=(const Coordinates& rhs) const {
    return !(*this == rhs);
}

double ComputeDistance(Coordinates from, Coordinates to) {
    using namespace std;
    
    const double dr = M_PI / 180.0;
    const int EARTH_RADIUS = 6371000; // в метрах
    
    return acos(sin(from.lat * dr) * sin(to.lat * dr)
                + cos(from.lat * dr) * cos(to.lat * dr) * cos(abs(from.lng - to.lng) * dr))
           * EARTH_RADIUS;
}

}  // namespace catalogue::geo
