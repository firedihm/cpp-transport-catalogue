#include "json_reader.h"

#include <iostream>
#include <sstream>

using namespace std::literals;

int main() {
    std::istringstream iss(R"({
      "base_requests": [
          {
              "is_roundtrip": true,
              "name": "289",
              "stops": [
                  "Zagorye",
                  "Lipetskaya ulitsa 46",
                  "Lipetskaya ulitsa 40",
                  "Lipetskaya ulitsa 40",
                  "Lipetskaya ulitsa 46",
                  "Moskvorechye",
                  "Zagorye"
              ],
              "type": "Bus"
          },
          {
              "latitude": 55.579909,
              "longitude": 37.68372,
              "name": "Zagorye",
              "road_distances": {
                  "Lipetskaya ulitsa 46": 230
              },
              "type": "Stop"
          },
          {
              "latitude": 55.581441,
              "longitude": 37.682205,
              "name": "Lipetskaya ulitsa 46",
              "road_distances": {
                  "Lipetskaya ulitsa 40": 390,
                  "Moskvorechye": 12400
              },
              "type": "Stop"
          },
          {
              "latitude": 55.584496,
              "longitude": 37.679133,
              "name": "Lipetskaya ulitsa 40",
              "road_distances": {
                  "Lipetskaya ulitsa 40": 1090,
                  "Lipetskaya ulitsa 46": 380
              },
              "type": "Stop"
          },
          {
              "latitude": 55.638433,
              "longitude": 37.638433,
              "name": "Moskvorechye",
              "road_distances": {
                  "Zagorye": 10000
              },
              "type": "Stop"
          }
      ],
      "render_settings": {
          "bus_label_font_size": 20,
          "bus_label_offset": [
              7,
              15
          ],
          "color_palette": [
              "green",
              [
                  255,
                  160,
                  0
              ],
              "red"
          ],
          "height": 200,
          "line_width": 14,
          "padding": 30,
          "stop_label_font_size": 20,
          "stop_label_offset": [
              7,
              -3
          ],
          "stop_radius": 5,
          "underlayer_color": [
              255,
              255,
              255,
              0.85
          ],
          "underlayer_width": 3,
          "width": 200
      },
      "routing_settings": {
          "bus_velocity": 30,
          "bus_wait_time": 2
      },
      "stat_requests": [
          {
              "id": 1,
              "name": "289",
              "type": "Bus"
          },
          {
              "from": "Zagorye",
              "id": 2,
              "to": "Moskvorechye",
              "type": "Route"
          },
          {
              "from": "Moskvorechye",
              "id": 3,
              "to": "Zagorye",
              "type": "Route"
          },
          {
              "from": "Lipetskaya ulitsa 40",
              "id": 4,
              "to": "Lipetskaya ulitsa 40",
              "type": "Route"
          }
      ]
    })"s);
    
    catalogue::TransportCatalogue catalogue;
    const json::Document& input = json::Load(iss);
    
    json::JsonReader reader(catalogue, input, std::cout);
    reader.ProcessBaseRequests();
    reader.PrintStats();
    //reader.RenderMap();
}
