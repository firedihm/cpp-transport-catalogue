#include "input_reader.h"
#include "transport_catalogue.h"

#include <algorithm>
#include <cassert>
#include <iterator>

namespace catalogue::input {
namespace detail {
/*
 * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
 */
geo::Coordinates ParseCoordinates(std::string_view str) {
    static const double nan = std::nan("");

    auto not_space = str.find_first_not_of(' ');
    auto comma = str.find(',', not_space);
    if (comma == str.npos) {
        return {nan, nan};
    }

    auto not_space2 = str.find_first_not_of(' ', comma + 1);
    auto comma2 = str.find(',', not_space2); // может быть npos

    double lat = std::stod(std::string(str.substr(not_space, str.find_last_not_of(' ', comma - 1) + 1 - not_space)));
    double lng = std::stod(std::string(str.substr(not_space2, str.find_last_not_of(' ', comma2 - 1) + 1 - not_space2)));

    return {lat, lng};
}

std::vector<std::pair<std::string_view, int>> ParseDistances(std::string_view str) {
    using namespace std::string_literals;
    std::vector<std::pair<std::string_view, int>> result;
    
    // пропускаем первые две запятые, которые отделяют координаты остановки
    auto comma = str.find(',');
    comma = str.find(',', comma + 1);
    
    while (comma != str.npos) {
        auto not_space = str.find_first_not_of(' ', comma + 1);
        auto m = str.find('m', not_space);
        
        auto not_space2 = str.find("to"s, m);
        not_space2 = str.find_first_not_of(' ', not_space2 + 2);
        comma = str.find(',', comma + 1); // может быть npos
        
        result.emplace_back(str.substr(not_space2, str.find_last_not_of(' ', comma - 1) + 1 - not_space2),
                            std::stoi(std::string(str.substr(not_space, m - not_space))));
    }
    
    return result;
}

/*
 * Удаляет пробелы в начале и конце строки
 */
std::string_view Trim(std::string_view string) {
    const auto start = string.find_first_not_of(' ');
    if (start == string.npos) {
        return {};
    }
    return string.substr(start, string.find_last_not_of(' ') + 1 - start);
}

/*
 * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
 */
std::vector<std::string_view> Split(std::string_view string, char delim) {
    std::vector<std::string_view> result;

    size_t pos = 0;
    while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
        auto delim_pos = string.find(delim, pos);
        if (delim_pos == string.npos) {
            delim_pos = string.size();
        }
        if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
            result.push_back(substr);
        }
        pos = delim_pos + 1;
    }

    return result;
}

/*
 * Парсит маршрут.
 * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
 * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
 */
std::vector<std::string_view> ParseRoute(std::string_view route) {
    if (route.find('>') != route.npos) {
        return Split(route, '>');
    }

    auto stops = Split(route, '-');
    std::vector<std::string_view> results(stops.begin(), stops.end());
    results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

    return results;
}

CommandDescription ParseCommandDescription(std::string_view line) {
    size_t colon = line.find(':');
    if (colon == line.npos) {
        return {};
    }

    size_t delim = line.find(' ', line.find_first_not_of(' '));
    if (delim >= colon) {
        return {};
    }

    size_t not_space = line.find_first_not_of(' ', delim);
    if (not_space >= colon) {
        return {};
    }

    return {std::string(line.substr(line.find_first_not_of(' '), delim)),
            std::string(line.substr(not_space, line.find_last_not_of(' ', colon - 1) + 1 - not_space)),
            std::string(line.substr(colon + 1))};
}
} // namespace detail

void InputReader::ParseLine(std::string_view line) {
    auto command_description = detail::ParseCommandDescription(line);
    if (command_description) {
        commands_.push_back(std::move(command_description));
    }
}

void InputReader::ApplyCommands(TransportCatalogue& catalogue) const {
    using namespace std::literals;
    
    // сначала инициализируем остановки...
    for (const auto& command : commands_) {
        if (command.command == "Stop"s) {
            catalogue.AddStop(command.id, detail::ParseCoordinates(command.description));
        }
    }
    
    // ...затем расстояния между ними...
    for (const auto& command : commands_) {
        if (command.command == "Stop"s) {
            catalogue.AddDistance(command.id, detail::ParseDistances(command.description));
        }
    }
    
    // ...потом маршруты
    for (const auto& command : commands_) {
        if (command.command == "Bus"s) {
            catalogue.AddBus(command.id, detail::ParseRoute(command.description));
        }
    }
}

} // namespace catalogue::input
