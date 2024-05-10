#pragma once

namespace catalogue::geo {

struct Coordinates {
    bool operator==(const Coordinates& rhs) const;
    bool operator!=(const Coordinates& rhs) const;
    
    double lat = 0.0;
    double lng = 0.0;
};

double ComputeDistance(Coordinates from, Coordinates to);

} // namespace catalogue::geo
