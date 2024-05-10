#pragma once

namespace geo {

struct Coordinates {
    inline bool operator==(const Coordinates& rhs) const { return lat == rhs.lat && lng == rhs.lng; }
    inline bool operator!=(const Coordinates& rhs) const { return !(*this == rhs); }
    
    double lat = 0.0;
    double lng = 0.0;
};

double ComputeDistance(Coordinates from, Coordinates to);

} // namespace geo
