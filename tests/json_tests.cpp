#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "../json.hpp"

struct TestResult {
    std::string name;
    std::string detail;
    bool passed;
};

void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void requireContains(const std::string& text, const std::string& expected, const std::string& message) {
    if (text.find(expected) == std::string::npos) {
        throw std::runtime_error(message);
    }
}

void requireNotContains(const std::string& text,
     const std::string& unexpected, const std::string& message) {
    if (text.find(unexpected) != std::string::npos) {
        throw std::runtime_error(message);
    }
}

class TestRunner {
public:
    void runTest(const std::string& name, const std::function<void()>& test) {
        std::cout << "[RUN ] " << name << "\n";
        try {
            test();
            results.push_back({name, "", true});
            std::cout << "[PASS] " << name << "\n";
        } catch (const std::exception& ex) {
            results.push_back({name, ex.what(), false});
            std::cout << "[FAIL] " << name << " - " << ex.what() << "\n";
        }
    }

    int printSummary() const {
        int failed = 0;
        for (const TestResult& result : results) {
            if (!result.passed) {
                failed++;
            }
        }
        std::cout << "\nSummary: " << (results.size() - failed) << " passed, "
                  << failed << " failed, " << results.size() << " total.\n";
        return failed;
    }

private:
    std::vector<TestResult> results;
};


void runAll(TestRunner& runner) {
    runner.runTest("Validate valid JSON", []() { testValidateValidJson(); });
    runner.runTest("Validate invalid JSON", []() { testValidateInvalidJson(); });
    runner.runTest("Search finds nested keys", []() { testSearchFindsNestedKeys(); });
    runner.runTest("Create value on empty root", []() { testCreateValueOnEmptyRoot(); });
    runner.runTest("Set path updates value", []() { testSetPathValueUpdates(); });
    runner.runTest("Delete array element", []() { testDeleteArrayElement(); });
    runner.runTest("Move value to new key", []() { testMoveValueMovesKey(); });
    runner.runTest("Delete invalid path leaves data unchanged", []() { testDeleteInvalidPath(); });
}

// Validates that well-formed JSON parses successfully
void testValidateValidJson() {
    JsonParser parser;
    parser.setJsonData("{\"name\":\"Alice\",\"age\":30}");
    bool ok = parser.validate(true);
    std::string data = parser.getData(0);
    require(ok, "Expected valid JSON to validate.");
    requireContains(data, "\"name\"", "Expected key in JSON output.");
    requireContains(data, "\"Alice\"", "Expected value in JSON output.");
}

// Validates that malformed JSON is rejected with a message
void testValidateInvalidJson() {
    JsonParser parser;
    parser.setJsonData("{\"name\": ");
    bool ok = parser.validate(true);
    require(!ok, "Expected invalid JSON to fail validation.");
    std::string data = parser.getData(0);
    require(data.empty(), "Expected no JSON data after invalid validation.");
}

// Ensures search returns values from nested objects
void testSearchFindsNestedKeys() {
    JsonParser parser;
    parser.setJsonData("{\"name\":\"A\",\"nested\":{\"name\":\"B\"}}");
    std::ostringstream output;
    std::streambuf* previousBuffer = std::cout.rdbuf(output.rdbuf());
    parser.search("name");
    std::cout.rdbuf(previousBuffer);

    std::string text = output.str();
    requireContains(text, "Search results for key 'name':", "Expected search output header.");
    requireContains(text, "\"A\"", "Expected to find first name value.");
    requireContains(text, "\"B\"", "Expected to find second name value.");
}

// Creates a value on an empty root and checks serialization
void testCreateValueOnEmptyRoot() {
    JsonParser parser;
    std::string value = "X";
    parser.createValue("parent/child", value);
    std::string data = parser.getData(0);
    requireContains(data, "\"parent\"", "Expected parent key in JSON output.");
    requireContains(data, "\"child\"", "Expected child key in JSON output.");
    requireContains(data, "\"X\"", "Expected created value in JSON output.");
}

// Replaces an existing value at a path
void testSetPathValueUpdates() {
    JsonParser parser;
    parser.setJsonData("{\"parent\":{\"child\":\"old\"}}");
    std::string value = "new";
    parser.setPathValue("parent/child", value);
    std::string data = parser.getData(0);
    requireContains(data, "\"new\"", "Expected updated value in JSON output.");
    requireNotContains(data, "\"old\"", "Expected old value to be removed.");
}

// Deletes an element from an array by index
void testDeleteArrayElement() {
    JsonParser parser;
    parser.setJsonData("{\"arr\":[9,2,7]}");
    parser.deleteValue("arr/1");
    std::string data = parser.getData(0);
    requireNotContains(data, "\n        2", "Expected the array element to be removed.");
}

// Moves a key to a new destination key
void testMoveValueMovesKey() {
    JsonParser parser;
    parser.setJsonData("{\"a\":1,\"b\":2}");
    parser.moveValue("a", "c");
    std::string data = parser.getData(0);
    requireContains(data, "\"c\": 1", "Expected the destination key to exist.");
    requireNotContains(data, "\"a\": 1", "Expected the source key to be removed.");
}

// Verifies that deleting a missing path leaves JSON unchanged
void testDeleteInvalidPath() {
    JsonParser parser;
    parser.setJsonData("{\"a\":1}");
    std::string before = parser.getData(0);
    parser.deleteValue("missing");
    std::string after = parser.getData(0);
    require(before == after, "Expected JSON to remain unchanged for missing path.");
}

// Runs the JsonParser test suite.
int main() {
    TestRunner runner;
    runAll(runner);
    return runner.printSummary() == 0 ? 0 : 1;
}
