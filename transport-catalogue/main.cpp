#include "json_reader.h"

#include <iostream>

int main() {
    catalogue::TransportCatalogue catalogue;
    const json::Document& input = json::Load(std::cin);
    
    json::ParseAndPrintRequests(catalogue, input, std::cout);
    //json::ParseAndRenderMap(catalogue, input, std::cout);
}
