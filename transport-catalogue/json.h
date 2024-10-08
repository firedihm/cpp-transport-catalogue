#pragma once

#include <map>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

namespace json {

class Node;
using Array = std::vector<Node>;
using Dict = std::map<std::string, Node>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node final : std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string> {
public:
    using variant::variant;
    using Value = variant;
    
    explicit Node(Value&& value) : variant(std::move(value)) {}
    explicit Node(std::string_view sv) : variant(std::string(sv)) {}
    
    inline bool operator==(const Node& rhs) const { return GetValue() == rhs.GetValue(); }
    inline bool operator!=(const Node& rhs) const { return !(*this == rhs); }
    
    inline const Value& GetValue() const { return *this; }
    inline Value& GetValue() { return *this; }
    
    bool IsInt() const;
    bool IsPureDouble() const;
    bool IsDouble() const;
    bool IsBool() const;
    bool IsString() const;
    bool IsNull() const;
    bool IsArray() const;
    bool IsMap() const;
    
    int AsInt() const;
    double AsDouble() const;
    bool AsBool() const;
    const std::string& AsString() const;
    const Array& AsArray() const;
    const Dict& AsMap() const;
    std::string& AsString();
    Array& AsArray();
    Dict& AsMap();
};

class Document {
public:
    explicit Document(Node&& root) : root_(std::move(root)) {}
    
    inline bool operator==(const Document& rhs) const { return root_ == rhs.root_; }
    inline bool operator!=(const Document& rhs) const { return !(*this == rhs); }
    
    inline const Node& GetRoot() const { return root_; }
    
private:
    Node root_;
};

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output, int step, int indent);

}  // namespace json
