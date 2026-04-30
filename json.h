#include <vector>
#include <map>
#include <stack>

// Abstract base node for all JSON elements.
class JsonNode {
public:
    // Virtual destructor for safe polymorphic cleanup.
    virtual ~JsonNode() = default;
    // Prints the node to stdout with indentation.
    virtual void print(unsigned int indent = 0) const = 0;
    // Emits indentation for pretty printing.
    void printIndent(unsigned int indent) const {
        for (int i = 0; i < indent; i++) std::cout << "  ";
    }
    // Serializes the node to a formatted string.
    virtual std::string getData(unsigned int indent = 0) const = 0;
    // Builds an indentation string for serialization.
    std::string getIndent(unsigned int indent) const {
        std::string data;
        for (int i = 0; i < indent; i++) {
            data += "    ";
        }
        return data;
    }
};

// Represents primitive JSON values such as strings, numbers, and literals.
class JsonPrimitive : public JsonNode {
private:
    std::string value;
    friend class JsonParser;

public:
    // Stores the raw value text.
    JsonPrimitive(const std::string& val) : value(val) {}

    // Prints the primitive value.
    void print(unsigned int indent = 0) const {
        std::cout << value;
    }

    // Returns the serialized primitive value.
    std::string getData(unsigned int indent = 0) const {
        return value;
    }
};

// Represents a JSON object (key-value mapping).
class JsonObject : public JsonNode {
private:
    std::map<std::string, JsonNode*> members;
    friend class JsonParser;

public:
    // Deletes all member nodes.
    ~JsonObject() {
        for (auto const& pair : members) {
            delete pair.second;
        }
    }

    // Prints the object with pretty formatting.
    void print(unsigned int indent = 0) const {
        unsigned int nextIndent = indent + 1;
        std::cout << "{\n";
        bool first = true;
        for (auto const& pair : members) {
            if (!first) std::cout << ",\n";
            printIndent(nextIndent);
            std::cout << "\"" << pair.first << "\": ";
            pair.second->print(nextIndent);
            first = false;
        }
        std::cout << "\n";
        printIndent(indent);
        std::cout << "}";
    }

    // Returns a formatted JSON object string.
    std::string getData(unsigned int indent = 0) const {
        std::string data;
        unsigned int nextIndent = indent + 1;
        data += "{\n";
        bool first = true;
        for (auto const& pair : members) {
            if (!first) data += ",\n";
            data += getIndent(nextIndent) + '\"' + pair.first + "\": " + 
                pair.second->getData(nextIndent);
            first = false;
        }
        data += '\n' + getIndent(indent) + '}';
        return data;
    }
};

// Represents a JSON array (ordered list).
class JsonArray : public JsonNode {
private:
    std::vector<JsonNode*> elements;
    friend class JsonParser;

public:
    // Deletes all element nodes.
    ~JsonArray() {
        for (JsonNode* node : elements) {
            delete node;
        }
    }

    // Prints the array with pretty formatting.
    void print(unsigned int indent = 0) const {
        unsigned int nextIndent = indent + 1;
        std::cout << "[\n";
        bool first = true;
        for (JsonNode* node : elements) {
            if (!first) std::cout << ",\n";
            printIndent(nextIndent);
            node->print(nextIndent);
            first = false;
        }
        std::cout << "\n";
        printIndent(indent);
        std::cout << "]";
    }

    // Returns a formatted JSON array string.
    std::string getData(unsigned int indent = 0) const {
        std::string data;
        unsigned int nextIndent = indent + 1;
        data += "[\n";
        bool first = true;
        for (JsonNode* node : elements) {
            if (!first) data += ",\n";
            data += getIndent(nextIndent) + node->getData(nextIndent);
            first = false;
        }
        data += '\n' + getIndent(indent) + ']';
        return data;
    }
};


// Parses, validates, and mutates JSON data in memory.
class JsonParser {
public:
    // Releases the parsed tree.
    ~JsonParser() {
        delete root;
    }

    // Sets raw JSON text and performs a silent validation.
    void setJsonData(const std::string& jsonData) {
        this->jsonData = jsonData;
        validate(false);
    }

    // Validates JSON text and optionally prints messages.
    bool validate(bool showMessages) {
        delete root;
        std::stringstream ss(jsonData);
        try {
            root = parseValue(ss);
            skipWhitespace(ss);
            if (ss.good() && !ss.eof()) {
                printMessage(showMessages, "Invalid JSON: Unexpected trailing characters.");
                return false;
            }
            printMessage(showMessages, "Valid JSON.");
            return true;
        } catch (const std::exception& e) {
            printMessage(showMessages, e.what());
            delete root;
            return false;
        }
    }

    // Prints the parsed JSON tree or an error message.
    void print() {
        if (root) {
            root->print();
            std::cout << "\n";
        } else {
            std::cout << "No valid JSON data loaded.\n";
        }
    }

    // Searches for values by key and prints all matches.
    void search(const std::string& key) {
        std::vector<JsonNode*> results;
        searchRecursive(root, key, results);
        
        std::cout << "Search results for key '" << key << "': [\n";
        bool first = true;
        for (size_t i = 0; i < results.size(); ++i) {
            results[i]->print(1);
            if (!first) std::cout << ",";
            std::cout << "\n";
            first = false;
        }
        std::cout << "]\n";
    }

    // Replaces a value at an existing path.
    void setPathValue(const std::string& path, std::string& value) {
        std::vector<std::string> parts = splitPath(path);
        if (parts.empty()) return;

        JsonNode* parent = findParentNode(root, parts, 0);
        std::string finalKey = parts.back();

        JsonObject* obj = dynamic_cast<JsonObject*>(parent);
        JsonArray* arr = dynamic_cast<JsonArray*>(parent);

        formatValue(value);
        if (obj && obj->members.find(finalKey) != obj->members.end()) {
            delete obj->members[finalKey];
            obj->members[finalKey] = new JsonPrimitive(value);
            std::cout << "Set " << path << " with the value " << value << std::endl;
            return;
            
        } else if (arr) {
            size_t index = std::stoi(finalKey);
            if (index < arr->elements.size()) {
                delete arr->elements[index];
                arr->elements[index] = new JsonPrimitive(value);
                std::cout << "Set " << path << " with the value " << value << std::endl;
                return;
            }
        }
        std::cout << "Invalid Path: '" << path << "' does not exist.\n";
    }

    // Creates a new value at a path, expanding objects as needed.
    void createValue(const std::string& path, std::string& value) {
        std::vector<std::string> parts = splitPath(path);
        if (parts.empty()) return;

        if (!root) {
            root = new JsonObject();
        }

        JsonNode* current = root;

        for (size_t i = 0; i < parts.size() - 1; ++i) {
            const std::string& key = parts[i];
            JsonObject* obj = dynamic_cast<JsonObject*>(current);
            if (!obj) {
                std::cout << "Invalid Path: Path '" << path << "' does not exist.\n";
                return;
            }

            auto it = obj->members.find(key);
            if (it == obj->members.end()) {
                JsonObject* next = new JsonObject();
                obj->members[key] = next;
                current = next;
            } else {
                current = it->second;
            }
        }

        const std::string& finalKey = parts.back();
        JsonObject* obj = dynamic_cast<JsonObject*>(current);
        JsonArray* arr = dynamic_cast<JsonArray*>(current);

        formatValue(value);
        if (obj) {
            if (obj->members.find(finalKey) == obj->members.end()) {
                obj->members[finalKey] = new JsonPrimitive(value);
                std::cout << "Created " << path << " with value " << value << std::endl;
                return;
            }
            std::cout << "Invalid Path: Path '" << path << "' already exists.\n";
            return;

        } else if (arr) {
            arr->elements.push_back(new JsonPrimitive(value));
            std::cout << "Added " << value << " to " << path << std::endl;
            return;
        }

        std::cout << "Invalid Path: Path '" << path << "' does not exist.\n";
    }

    // Deletes a value at the given path.
    void deleteValue(const std::string& path) {
        std::vector<std::string> parts = splitPath(path);
        if (parts.empty()) return;

        JsonNode* parent = findParentNode(root, parts, 0);
        std::string finalKey = parts.back();

        JsonObject* obj = dynamic_cast<JsonObject*>(parent);
        JsonArray* arr = dynamic_cast<JsonArray*>(parent);

        if (obj) {
            auto itEle = obj->members.find(finalKey);
            if (itEle != obj->members.end()) {
                delete itEle->second;
                obj->members.erase(itEle);
                std::cout << "Deleted " << path << std::endl;
                return;
            }
        } else if (arr) {
            size_t index = std::stoi(finalKey);
            if (index < arr->elements.size()) {
                delete arr->elements[index];
                arr->elements.erase(arr->elements.begin() + index);
                std::cout << "Deleted " << path << std::endl;
                return;
            }
        }
        std::cout << "Error: Path '" << path << "' does not exist.\n";
    }

    // Moves a value from one path to another.
    void moveValue(const std::string& fromPath, const std::string& toPath) {
        std::vector<std::string> fromParts = splitPath(fromPath);
        if (fromParts.empty()) return;
        JsonNode* fromParent = findParentNode(root, fromParts, 0);
        std::string fromKey = fromParts.back();
        
        JsonNode* targetNode = nullptr;
        JsonObject* fromObj = dynamic_cast<JsonObject*>(fromParent);
        if (fromObj && fromObj->members.find(fromKey) != fromObj->members.end()) {
            targetNode = fromObj->members[fromKey];
            fromObj->members.erase(fromKey);
        }

        if (!targetNode) {
            std::cout << "Invalid Path: From path '" << fromPath << "' does not exist.\n";
            return;
        }

        std::vector<std::string> toParts = splitPath(toPath);
        JsonNode* toParent = findParentNode(root, toParts, 0);
        std::string toKey = toParts.back();

        JsonObject* toObj = dynamic_cast<JsonObject*>(toParent);
        if (toObj && toObj->members.find(toKey) == toObj->members.end()) {
            toObj->members[toKey] = targetNode;
            std::cout << "Moved from " << fromPath << " to " << toPath << std::endl;
            return;
        }
        
        delete targetNode; 
        std::cout << "Invalid Path: Destination path is invalid or already occupied.\n";
    }

    // Serializes the current JSON tree with indentation.
    std::string getData(unsigned int indent) const {
        if (root) {
            return root->getData(indent);

        } else {
            return std::string("");
        }
    }

private:
    JsonNode* root = nullptr;
    std::string jsonData;

    // Normalizes value as a JSON literal or quoted string.
    void formatValue(std::string& value) const {
        bool isNum = false;
        try {
            size_t parsedSize = 0;
            std::stod(value, &parsedSize);
            isNum = parsedSize == value.size();
        } catch (const std::exception&) {}

        if (!isNum || value != "true" || value != "false" || value != "null") {
            value = '\"' + value + '\"';
        }
    }

    // Prints a message only when user feedback is enabled.
    void printMessage(bool showMessages, const char* message) {
        if (showMessages) {
            std::cout << message << std::endl;
        }
    }

    // Consumes whitespace in the stream.
    void skipWhitespace(std::stringstream& ss) {
        ss >> std::ws;
    }

    // Parses the next JSON value based on leading token.
    JsonNode* parseValue(std::stringstream& ss) {
        skipWhitespace(ss);
        char c = ss.peek();

        if (c == '{') return parseObject(ss);
        if (c == '[') return parseArray(ss);
        if (c == '"') return formatString(ss);
        return parseOther(ss);
    }

    // Parses a JSON object into a node tree.
    JsonNode* parseObject(std::stringstream& ss) {
        JsonObject* obj = new JsonObject();
        ss.ignore();
        skipWhitespace(ss);

        if (ss.peek() == '}') {
            ss.ignore();
            return obj;
        }

        while (ss.good()) {
            std::string key = parseString(ss);
            skipWhitespace(ss);
            
            if (ss.get() != ':') throw std::runtime_error("Expected ':'.");
            
            obj->members[key] = parseValue(ss);
            
            skipWhitespace(ss);
            char next = ss.get();
            if (next == '}') break;
            if (next != ',') throw std::runtime_error("Expected ',' or '}'.");
            skipWhitespace(ss);
        }
        return obj;
    }

    // Parses a JSON array into a node tree.
    JsonNode* parseArray(std::stringstream& ss) {
        JsonArray* arr = new JsonArray();
        ss.ignore();
        skipWhitespace(ss);

        if (ss.peek() == ']') {
            ss.ignore();
            return arr;
        }

        while (ss.good()) {
            arr->elements.push_back(parseValue(ss));
            skipWhitespace(ss);
            char next = ss.get();
            if (next == ']') break;
            if (next != ',') throw std::runtime_error("Expected ',' or ']'.");
        }
        return arr;
    }

    // Parses a quoted JSON string without the quotes.
    std::string parseString(std::stringstream& ss) {
        std::string out;
        if (ss.get() != '"') throw std::runtime_error("Expected string starting with '\"'.");
        std::getline(ss, out, '"');
        return out;
    }

    // Builds a primitive node from a parsed string.
    JsonNode* formatString(std::stringstream& ss) {
        return new JsonPrimitive("\"" + parseString(ss) + "\"");
    }

    // Parses numbers and literal values (true/false/null).
    JsonNode* parseOther(std::stringstream& ss) {
        std::string val;
        char c = ss.peek();
        while (ss.good() && !std::isspace(c) && c != ',' && 
               c != '}' && c != ']') {
            val += static_cast<char>(ss.get());
            c = ss.peek();
        }

        if (val.empty()) throw std::runtime_error("Unexpected value.");

        if (val == "true" || val == "false" || val == "null") {
            return new JsonPrimitive(val);
        }

        size_t parsedSize = 0;
        std::stod(val, &parsedSize);
        if (parsedSize != val.length()) {
            throw std::runtime_error("Unexpected value.");
        }
        return new JsonPrimitive(val);

    }

    // Splits a slash-delimited path into parts.
    std::vector<std::string> splitPath(const std::string& path) {
        std::vector<std::string> parts;
        std::stringstream ss(path);
        std::string part;
        while (std::getline(ss, part, '/')) {
            parts.push_back(part);
        }
        return parts;
    }

    // Walks the tree to find the parent of a target path.
    JsonNode* findParentNode(JsonNode* current, const std::vector<std::string>& path, size_t index) {
        if (!current || index == path.size() - 1) {
            return current;
        }
        JsonObject* obj = dynamic_cast<JsonObject*>(current);
        JsonArray* arr = dynamic_cast<JsonArray*>(current);
        if (obj && obj->members.find(path[index]) != obj->members.end()) {
            return findParentNode(obj->members[path[index]], path, index + 1);

        } else if (arr) {
            size_t i = std::stoi(path[index]);
            if (i < arr->elements.size()) {
                return findParentNode(arr->elements[i], path, index + 1);
            }
        }
        return nullptr;
    }

    // Collects all values that match a specific key.
    void searchRecursive(JsonNode* current, const std::string& searchKey, std::vector<JsonNode*>& results) {
        if (!current) return;

        JsonObject* obj = dynamic_cast<JsonObject*>(current);
        JsonArray* arr = dynamic_cast<JsonArray*>(current);

        if (obj) {
            for (auto const& pair : obj->members) {
                if (pair.first == searchKey) {
                    results.push_back(pair.second);
                }
                searchRecursive(pair.second, searchKey, results);
            }
        } else if (arr) {
            for (JsonNode* node : arr->elements) {
                searchRecursive(node, searchKey, results);
            }
        }
    }
};