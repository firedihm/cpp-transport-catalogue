#include "json_builder.h"

using namespace std::literals;

namespace json {

Builder::KeyContext Builder::Key(std::string&& key) {
    if (nodes_stack_.empty() || !nodes_stack_.top()->IsMap()) {
        throw std::logic_error("Dictionary must be created before adding key "s + key);
    }
    
    nodes_stack_.push(new Node(std::move(key)));
    return BaseContext(*this);
}

Builder::ValueContext Builder::Value(Node::Value&& value) {
    if (!root_.IsNull() && !nodes_stack_.empty() &&
        !nodes_stack_.top()->IsArray() && !nodes_stack_.top()->IsString()) {
        throw std::logic_error("Array or dictionary key must be created before adding any values"s);
    }
    
    AddNode(std::move(value));
    return BaseContext(*this);
}

Builder::DictContext Builder::StartDict() {
    if (!nodes_stack_.empty() && nodes_stack_.top()->IsMap()) {
        throw std::logic_error("Dictionary cannot be key value of another dictionary"s);
    }
    
    nodes_stack_.push(AddNode(Dict()));
    return BaseContext(*this);
}

Builder::BaseContext Builder::EndDict() {
    if (nodes_stack_.empty() || !nodes_stack_.top()->IsMap()) {
        throw std::logic_error("Dictionary's key-value pair must be defined before it can be closed"s);
    }
    
    nodes_stack_.pop();
    return *this;
}

Builder::ArrayContext Builder::StartArray() {
    if (!nodes_stack_.empty() && nodes_stack_.top()->IsMap()) {
        throw std::logic_error("Array cannot be key value of dictionary"s);
    }
    
    nodes_stack_.push(AddNode(Array()));
    return BaseContext(*this);
}

Builder::BaseContext Builder::EndArray() {
    if (nodes_stack_.empty() || !nodes_stack_.top()->IsArray()) {
        throw std::logic_error("Array's elements must be defined before it can be closed"s);
    }
    
    nodes_stack_.pop();
    return *this;
}

Node Builder::Build() {
    if (root_.IsNull() || !nodes_stack_.empty()) {
        throw std::logic_error("All arrays and dictionaries must be closed before building JSON"s);
    }
    
    return root_;
}

Node* Builder::AddNode(Node::Value&& value) {
    if (root_.IsNull()) {
        root_ = Node(std::move(value));
        return &root_;
    } else if (nodes_stack_.empty()) {
        throw std::logic_error("JSON file cannot store additional nodes if it was initialised with value"s);
    } else if (nodes_stack_.top()->IsArray()) {
        Node& node = nodes_stack_.top()->AsArray().emplace_back(std::move(value));
        return &node;
    } else if (nodes_stack_.top()->IsString()) {
        Node* key = nodes_stack_.top();
        nodes_stack_.pop();
        
        Node& new_node = nodes_stack_.top()->AsMap()[std::move(key->AsString())];
        new_node.GetValue() = std::move(value);
        delete key;
        return &new_node;
    } else {
        // это должно быть недостижимым условием
        return nullptr;
    }
}

} // namespace json
