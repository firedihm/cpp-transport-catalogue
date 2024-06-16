#include "json.h"

#include <iomanip>

using namespace std::literals;

namespace json {

bool Node::IsInt() const {
    return std::holds_alternative<int>(GetValue());
}

bool Node::IsPureDouble() const {
    return std::holds_alternative<double>(GetValue());
}

bool Node::IsDouble() const {
    return IsInt() || IsPureDouble();
}

bool Node::IsBool() const {
    return std::holds_alternative<bool>(GetValue());
}

bool Node::IsString() const {
    return std::holds_alternative<std::string>(GetValue());
}

bool Node::IsNull() const {
    return std::holds_alternative<std::nullptr_t>(GetValue());
}

bool Node::IsArray() const {
    return std::holds_alternative<Array>(GetValue());
}

bool Node::IsMap() const {
    return std::holds_alternative<Dict>(GetValue());
}

int Node::AsInt() const {
    if (!IsInt()) {
        throw std::logic_error("Node has no int"s);
    }
    return std::get<int>(GetValue());
}

double Node::AsDouble() const {
    if (!IsDouble()) {
        throw std::logic_error("Node has no double"s);
    }
    return IsPureDouble() ? std::get<double>(GetValue()) : std::get<int>(GetValue());
}

bool Node::AsBool() const {
    if (!IsBool()) {
        throw std::logic_error("Node has no bool"s);
    }
    return std::get<bool>(GetValue());
}

const std::string& Node::AsString() const {
    if (!IsString()) {
        throw std::logic_error("Node has no string"s);
    }
    return std::get<std::string>(GetValue());
}

const Array& Node::AsArray() const {
    if (!IsArray()) {
        throw std::logic_error("Node has no array"s);
    }
    return std::get<Array>(GetValue());
}

const Dict& Node::AsMap() const {
    if (!IsMap()) {
        throw std::logic_error("Node has no dictionary"s);
    }
    return std::get<Dict>(GetValue());
}

std::string& Node::AsString() {
    if (!IsString()) {
        throw std::logic_error("Node has no string"s);
    }
    return std::get<std::string>(GetValue());
}

Array& Node::AsArray() {
    if (!IsArray()) {
        throw std::logic_error("Node has no array"s);
    }
    return std::get<Array>(GetValue());
}

Dict& Node::AsMap() {
    if (!IsMap()) {
        throw std::logic_error("Node has no dictionary"s);
    }
    return std::get<Dict>(GetValue());
}

namespace detail {
Node LoadNode(std::istream& input);

Node LoadArray(std::istream& input) {
    Array result;
    for (char c; input >> c && c != ']';) {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }
    if (!input) {
        throw ParsingError("Array parsing error"s);
    }
    return Node(move(result));
}

Node LoadNull(std::istream& input) {
    std::string s;
    while (isalpha(input.peek())) {
        s.push_back(input.get());
    }
    
    if (s == "null"sv) {
        return Node(nullptr);
    } else {
        throw ParsingError("Failed to parse '"s + s + "' as null"s);
    }
}

Node LoadBool(std::istream& input) {
    std::string s;
    while (isalpha(input.peek())) {
        s.push_back(input.get());
    }
    
    if (s == "true"sv) {
        return Node(true);
    } else if (s == "false"sv) {
        return Node(false);
    } else {
        throw ParsingError("Failed to parse '"s + s + "' as bool"s);
    }
}

Node LoadNumber(std::istream& input) {
    std::string parsed_num;
    
    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Number parsing error"s);
        }
    };
    
    // Считывает одну или более цифр в parsed_num из input
    auto read_digits = [&input, read_char] {
        if (!isdigit(input.peek())) {
            throw ParsingError("Number parsing error: a digit is expected"s);
        }
        while (isdigit(input.peek())) {
            read_char();
        }
    };
    
    if (input.peek() == '-') {
        read_char();
    }
    // Парсим целую часть числа
    if (input.peek() == '0') {
        read_char();
        // После 0 в JSON не могут идти другие цифры
    } else {
        read_digits();
    }
    
    bool is_int = true;
    // Парсим дробную часть числа
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }
    
    // Парсим экспоненциальную часть числа
    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }
    
    try {
        if (is_int) {
            // Сначала пробуем преобразовать строку в int
            try {
                return Node(stoi(parsed_num));
            } catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
        return Node(stod(parsed_num));
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

Node LoadString(std::istream& input) {
    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string s;
    while (true) {
        if (it == end) {
            // Поток закончился до того, как встретили закрывающую кавычку?
            throw ParsingError("String parsing error"s);
        }
        const char ch = *it;
        if (ch == '"') {
            // Встретили закрывающую кавычку
            ++it;
            break;
        } else if (ch == '\\') {
            // Встретили начало escape-последовательности
            ++it;
            if (it == end) {
                // Поток завершился сразу после символа обратной косой черты
                throw ParsingError("String parsing error"s);
            }
            const char escaped_char = *(it);
            // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
            switch (escaped_char) {
                case 'n':
                    s.push_back('\n');
                    break;
                case 't':
                    s.push_back('\t');
                    break;
                case 'r':
                    s.push_back('\r');
                    break;
                case '"':
                    s.push_back('"');
                    break;
                case '\\':
                    s.push_back('\\');
                    break;
                default:
                    // Встретили неизвестную escape-последовательность
                    throw ParsingError("String parsing error: unrecognized escape sequence \\"s + escaped_char);
            }
        } else if (ch == '\n' || ch == '\r') {
            // Строковый литерал внутри- JSON не может прерываться символами \r или \n
            throw ParsingError("String parsing error: unexpected end of line"s);
        } else {
            // Просто считываем очередной символ и помещаем его в результирующую строку
            s.push_back(ch);
        }
        ++it;
    }
    return Node(move(s));
}

Node LoadDict(std::istream& input) {
    Dict result;
    for (char c; input >> c && c != '}';) {
        if (c == '"') {
            std::string key = LoadString(input).AsString();
            
            for (; input >> c && c != ':';) {}
            if (!input) {
                throw ParsingError("Dictionary parsing error: expected \':\' after key string"s);
            }
            result.emplace(std::move(key), LoadNode(input));
        } else if (c != ',') {
            throw ParsingError("Dictionary parsing error: key-value pair must start with string"s);
        }
    }
    if (!input) {
        throw ParsingError("Dictionary parsing error"s);
    }
    return Node(move(result));
}

Node LoadNode(std::istream& input) {
    char c;
    for (; input >> c && isspace(c);) {}
    if (!input) {
        throw ParsingError("Node parsing error"s);
    }
    
    switch (c) {
        case '[':
            return LoadArray(input);
        case '{':
            return LoadDict(input);
        case '"':
            return LoadString(input);
        case 'n':
            input.putback(c);
            return LoadNull(input);
        case 't':
        case 'f':
            input.putback(c);
            return LoadBool(input);
        default:
            input.putback(c);
            return LoadNumber(input);
    }
}
} // namespace detail

Document Load(std::istream& input) {
    return Document(detail::LoadNode(input));
}

class PrintContext {
public:
    PrintContext(std::ostream& out, int step, int indent) : out(out), step(step), indent(indent) {}
    
    PrintContext Indented() const { return PrintContext(out, step, step + indent); }
    void PrintIndent() const { out << std::setw(indent) << (indent ? " "sv : ""sv); }
    
    void PrintNode(const Node& node) {
        std::visit([this](const auto& value) { PrintValue(value); }, node.GetValue());
    }
    
private:
    void PrintValue(const bool value) { out << std::boolalpha << value; }
    void PrintValue(const int value) { out << value; }
    void PrintValue(const double value) { out << value; }
    void PrintValue(const nullptr_t) { out << "null"sv; }
    void PrintValue(const std::string& value) {
        out << '"';
        for (const char c : value) {
            switch (c) {
                case '\t':
                    out << "\\t"sv;
                    break;
                case '\n':
                    out << "\\n"sv;
                    break;
                case '\r':
                    out << "\\r"sv;
                    break;
                case '"':
                case '\\':
                    out << '\\';
                    [[fallthrough]];
                default:
                    out << c;
            }
        }
        out << '"';
    }
    void PrintValue(const Array& value) {
        PrintContext nested_ctx = Indented();
        
        out << "[\n"sv;
        if (!value.empty()) {
            auto it = value.begin();
            nested_ctx.PrintIndent();
            nested_ctx.PrintNode(*it);
            while (++it != value.end()) {
                out << ",\n"sv;
                nested_ctx.PrintIndent();
                nested_ctx.PrintNode(*it);
            }
        }
        out << '\n';
        PrintIndent();
        out << ']';
    }
    void PrintValue(const Dict& value) {
        PrintContext nested_ctx = Indented();
        
        out << "{\n"sv;
        if (!value.empty()) {
            auto it = value.begin();
            nested_ctx.PrintIndent();
            nested_ctx.PrintValue(it->first); // std::string
            out << ": "sv;
            nested_ctx.PrintNode(it->second); // Node
            while (++it != value.end()) {
                out << ",\n"sv;
                nested_ctx.PrintIndent();
                nested_ctx.PrintValue(it->first);
                out << ": "sv;
                nested_ctx.PrintNode(it->second);
            }
        }
        out << '\n';
        PrintIndent();
        out << '}';
    }
    
    std::ostream& out;
    int step;
    int indent;
};

void Print(const Document& doc, std::ostream& output, int step, int indent) {
    PrintContext ctx(output, step, indent);
    ctx.PrintNode(doc.GetRoot());
    output << '\n';
}

}  // namespace json
