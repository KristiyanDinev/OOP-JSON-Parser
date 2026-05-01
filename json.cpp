#pragma once
#include <cctype>
#include <exception>
#include <iostream>
#include <sstream>
#include <string>

#include "json.hpp"

// Virtual destructor for JsonNode
JsonNode::~JsonNode() = default;


// Default function to print the indent for every class
void JsonNode::printIndent(unsigned int indent) const {
    for (int i = 0; i < indent; i++) std::cout << "  ";
}

// Get indent as string
std::string JsonNode::getIndent(unsigned int indent) const {
    std::string data;
    for (int i = 0; i < indent; i++) {
        data += "    ";
    }
    return data;
}

// Constructor for JsonPrimitive, which keeps the raw values
JsonPrimitive::JsonPrimitive(const std::string& val) : value(val) {}

// Print that value
void JsonPrimitive::print(unsigned int indent) const {
    std::cout << value;
}

// Get that value
std::string JsonPrimitive::getData(unsigned int indent) const {
    return value;
}

// Virtual destructor for JsonObject
JsonObject::~JsonObject() {
    for (auto const& pair : members) {
        delete pair.second;
    }
}

// Pretty print JsonObject
void JsonObject::print(unsigned int indent) const {
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

// Get pretty print as string
std::string JsonObject::getData(unsigned int indent) const {
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

// Virtual destructor JsonArray
JsonArray::~JsonArray() {
    for (JsonNode* node : elements) {
        delete node;
    }
}

// Pretty print JsonArray
void JsonArray::print(unsigned int indent) const {
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

// Get pretty print as string
std::string JsonArray::getData(unsigned int indent) const {
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

// Virtual destructor for JsonParser, which deletes the root object from RAM
JsonParser::~JsonParser() {
    delete root;
}

// Set the JSON string to be parsed
void JsonParser::setJsonData(const std::string& jsonData) {
    this->jsonData = jsonData;
    validate(false);
}

// Validates JSON text with optional bool flag showMessages for feedback
bool JsonParser::validate(bool showMessages) {
    delete root;
    std::stringstream ss(jsonData);
    try {
        root = parseValue(ss);
        skipWhitespace(ss);
        printMessage(showMessages, "Valid JSON.");
        return true;
    } catch (const std::exception& e) {
        printMessage(showMessages, e.what());
        delete root;
        return false;
    }
}

// Print the root object
void JsonParser::print() {
    if (root) {
        root->print();
        std::cout << "\n";
    } else {
        std::cout << "No valid JSON data loaded.\n";
    }
}

// Search all values with that key. Paths can differ.
void JsonParser::search(const std::string& key) {
    std::vector<JsonNode*> results;
    searchRecursive(root, key, results);
    
    std::cout << "Search results for key '" << key << "': [\n";
    bool first = true;
    for (std::size_t i = 0; i < results.size(); ++i) {
        results[i]->print(1);
        if (!first) std::cout << ",";
        std::cout << "\n";
        first = false;
    }
    std::cout << "]\n";
}

// Replaces an already existing value on that path with our new value
void JsonParser::setPathValue(const std::string& path, std::string& value) {
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
        std::size_t index = std::stoi(finalKey);
        if (index < arr->elements.size()) {
            delete arr->elements[index];
            arr->elements[index] = new JsonPrimitive(value);
            std::cout << "Set " << path << " with the value " << value << std::endl;
            return;
        }
    }
    std::cout << "Invalid Path: '" << path << "' does not exist.\n";
}

// Creates a new value with the given path
void JsonParser::createValue(const std::string& path, std::string& value) {
    std::vector<std::string> parts = splitPath(path);
    if (parts.empty()) return;

    if (!root) {
        root = new JsonObject();
    }

    JsonNode* current = root;

    for (std::size_t i = 0; i < parts.size() - 1; ++i) {
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

// Deletes the value at the given path
void JsonParser::deleteValue(const std::string& path) {
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
        std::size_t index = std::stoi(finalKey);
        if (index < arr->elements.size()) {
            delete arr->elements[index];
            arr->elements.erase(arr->elements.begin() + index);
            std::cout << "Deleted " << path << std::endl;
            return;
        }
    }
    std::cout << "Error: Path '" << path << "' does not exist.\n";
}

// Moves a value from one path to another
void JsonParser::moveValue(const std::string& fromPath, const std::string& toPath) {
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

// Get the root pretty print as string 
std::string JsonParser::getData(unsigned int indent) const {
    if (root) {
        return root->getData(indent);

    } else {
        return std::string("");
    }
}

// Gets ready value for JsonPrimitive
void JsonParser::formatValue(std::string& value) const {
    bool isNum = false;
    try {
        std::size_t parsedSize = 0;
        std::stod(value, &parsedSize);
        isNum = parsedSize == value.size();
    } catch (const std::exception&) {}

    if (!isNum || value != "true" || value != "false" || value != "null") {
        value = '\"' + value + '\"';
    }
}

// Print a message only when showMessages is true
void JsonParser::printMessage(bool showMessages, const char* message) {
    if (showMessages) {
        std::cout << message << std::endl;
    }
}

// Skip white spaces
void JsonParser::skipWhitespace(std::stringstream& ss) {
    ss >> std::ws;
}

// Parses a JSON value
JsonNode* JsonParser::parseValue(std::stringstream& ss) {
    skipWhitespace(ss);
    char c = ss.peek();

    if (c == '{') return parseObject(ss);
    if (c == '[') return parseArray(ss);
    if (c == '"') return formatString(ss);
    return parseOther(ss);
}

// Parses a JSON object
JsonNode* JsonParser::parseObject(std::stringstream& ss) {
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
        
        if (ss.get() != ':') throw std::exception();
        
        obj->members[key] = parseValue(ss);
        
        skipWhitespace(ss);
        char next = ss.get();
        if (next == '}') break;
        if (next != ',') throw std::exception();
        skipWhitespace(ss);
    }
    return obj;
}

// Parses a JSON array
JsonNode* JsonParser::parseArray(std::stringstream& ss) {
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
        if (next != ',') throw std::exception();
    }
    return arr;
}

// Parses a JSON string
std::string JsonParser::parseString(std::stringstream& ss) {
    std::string out;
    if (ss.get() != '"') throw std::exception();
    std::getline(ss, out, '"');
    return out;
}

// Get JsonPrimitive for a string
JsonNode* JsonParser::formatString(std::stringstream& ss) {
    return new JsonPrimitive("\"" + parseString(ss) + "\"");
}

// Parses other than string values
JsonNode* JsonParser::parseOther(std::stringstream& ss) {
    std::string val;
    char c = ss.peek();
    while (ss.good() && !std::isspace(c) && c != ',' && 
           c != '}' && c != ']') {
        val += ss.get();
        c = ss.peek();
    }

    if (val.empty()) throw std::exception();

    if (val == "true" || val == "false" || val == "null") {
        return new JsonPrimitive(val);
    }

    std::size_t parsedSize = 0;
    std::stod(val, &parsedSize);
    if (parsedSize != val.length()) {
        throw std::exception();
    }
    return new JsonPrimitive(val);

}

// Splits the given path into a vector of keys
std::vector<std::string> JsonParser::splitPath(const std::string& path) {
    std::vector<std::string> parts;
    std::stringstream ss(path);
    std::string part;
    while (std::getline(ss, part, '/')) {
        parts.push_back(part);
    }
    return parts;
}

// Get the parent Object of a key
JsonNode* JsonParser::findParentNode(JsonNode* current, const std::vector<std::string>& path, std::size_t index) {
    if (!current || index == path.size() - 1) {
        return current;
    }
    JsonObject* obj = dynamic_cast<JsonObject*>(current);
    JsonArray* arr = dynamic_cast<JsonArray*>(current);
    if (obj && obj->members.find(path[index]) != obj->members.end()) {
        return findParentNode(obj->members[path[index]], path, index + 1);

    } else if (arr) {
        std::size_t i = std::stoi(path[index]);
        if (i < arr->elements.size()) {
            return findParentNode(arr->elements[i], path, index + 1);
        }
    }
    return nullptr;
}

// Get all values, which have a specific key
void JsonParser::searchRecursive(JsonNode* current, const std::string& searchKey, std::vector<JsonNode*>& results) {
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
