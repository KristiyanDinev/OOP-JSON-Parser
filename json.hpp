#pragma once

#include <map>
#include <sstream>
#include <stack>
#include <string>
#include <vector>

class JsonNode {
public:
    virtual ~JsonNode();
    virtual void print(unsigned int indent = 0) const = 0;
    void printIndent(unsigned int indent) const;
    virtual std::string getData(unsigned int indent = 0) const = 0;
    std::string getIndent(unsigned int indent) const;
};

class JsonPrimitive : public JsonNode {
private:
    std::string value;
    friend class JsonParser;

public:
    JsonPrimitive(const std::string& val);
    void print(unsigned int indent = 0) const;
    std::string getData(unsigned int indent = 0) const;
};

class JsonObject : public JsonNode {
private:
    std::map<std::string, JsonNode*> members;
    friend class JsonParser;

public:
    ~JsonObject();
    void print(unsigned int indent = 0) const;
    std::string getData(unsigned int indent = 0) const;
};

class JsonArray : public JsonNode {
private:
    std::vector<JsonNode*> elements;
    friend class JsonParser;

public:
    ~JsonArray();
    void print(unsigned int indent = 0) const;
    std::string getData(unsigned int indent = 0) const;
};

class JsonParser {
public:
    ~JsonParser();
    void setJsonData(const std::string& jsonData);
    bool validate(bool showMessages);
    void print();
    void search(const std::string& key);
    void setPathValue(const std::string& path, std::string& value);
    void createValue(const std::string& path, std::string& value);
    void deleteValue(const std::string& path);
    void moveValue(const std::string& fromPath, const std::string& toPath);
    std::string getData(unsigned int indent) const;

private:
    JsonNode* root = nullptr;
    std::string jsonData;

    void formatValue(std::string& value) const;
    void printMessage(bool showMessages, const char* message);
    void skipWhitespace(std::stringstream& ss);
    JsonNode* parseValue(std::stringstream& ss);
    JsonNode* parseObject(std::stringstream& ss);
    JsonNode* parseArray(std::stringstream& ss);
    std::string parseString(std::stringstream& ss);
    JsonNode* formatString(std::stringstream& ss);
    JsonNode* parseOther(std::stringstream& ss);
    std::vector<std::string> splitPath(const std::string& path);
    JsonNode* findParentNode(JsonNode* current, const std::vector<std::string>& path, std::size_t index);
    void searchRecursive(JsonNode* current, const std::string& searchKey, std::vector<JsonNode*>& results);
};