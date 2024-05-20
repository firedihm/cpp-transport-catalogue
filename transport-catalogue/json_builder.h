#include "json.h"

#include <stack>

namespace json {

class Builder {
    class BaseContext;
    class ValueContext;
    class KeyContext;
    class DictContext;
    class ArrayContext;
    
public:
    ValueContext Value(Node::Value&& value);
    DictContext StartDict();
    ArrayContext StartArray();
    
private:
    KeyContext Key(std::string&& key);
    BaseContext EndDict();
    BaseContext EndArray();
    Node Build();
    
    Node* AddNode(Node::Value&& value);
    
    Node root_;
    std::stack<Node*, std::vector<Node*>> nodes_stack_;
    
    class BaseContext {
    public:
        BaseContext(Builder& builder) : builder_(builder) {}
        KeyContext Key(std::string&& key) { return builder_.Key(std::move(key)); }
        BaseContext Value(Node::Value&& value) { return static_cast<BaseContext>(builder_.Value(std::move(value))); }
        DictContext StartDict() { return builder_.StartDict(); }
        BaseContext EndDict() { return builder_.EndDict(); }
        ArrayContext StartArray() { return builder_.StartArray(); }
        BaseContext EndArray() { return builder_.EndArray(); }
        Node Build() { return builder_.Build(); }
    private:
        Builder& builder_;
    };
    
    // только для случая вызова Value() сразу после конструктора Builder
    class ValueContext : public BaseContext {
    public:
        ValueContext(BaseContext base) : BaseContext(base) {}
        KeyContext Key(std::string&& key) = delete;
        BaseContext Value(Node::Value&& value) = delete;
        DictContext StartDict() = delete;
        BaseContext EndDict() = delete;
        ArrayContext StartArray() = delete;
        BaseContext EndArray() = delete;
        //Node Build();
    };
    
    class KeyContext : public BaseContext {
    public:
        KeyContext(BaseContext base) : BaseContext(base) {}
        KeyContext Key(std::string&& key) = delete;
        DictContext Value(Node::Value&& value) { return static_cast<DictContext>(BaseContext::Value(std::move(value))); }
        //DictContext StartDict();
        BaseContext EndDict() = delete;
        //ArrayContext StartArray();
        BaseContext EndArray() = delete;
        Node Build() = delete;
    };
    
    class DictContext : public BaseContext {
    public:
        DictContext(BaseContext base) : BaseContext(base) {}
        //KeyContext Key(std::string&& key);
        BaseContext Value(Node::Value&& value) = delete;
        DictContext StartDict() = delete;
        //BaseContext EndDict();
        ArrayContext StartArray() = delete;
        BaseContext EndArray() = delete;
        Node Build() = delete;
    };
    
    class ArrayContext : public BaseContext {
    public:
        ArrayContext(BaseContext base) : BaseContext(base) {}
        KeyContext Key(std::string&& key) = delete;
        ArrayContext Value(Node::Value&& value) { return static_cast<ArrayContext>(BaseContext::Value(std::move(value))); }
        //DictContext StartDict();
        BaseContext EndDict() = delete;
        //ArrayContext StartArray();
        //BaseContext EndArray();
        Node Build() = delete;
    };
};

} // namespace json
