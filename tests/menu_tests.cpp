#include <functional>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "../json.hpp"
#include "../file.hpp"
#include "../menu.hpp"

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

void requireContains(const std::string& text, 
    const std::string& expected, const std::string& message) {
    if (text.find(expected) == std::string::npos) {
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


bool fileExists(const std::string& path) {
    std::ifstream file(path);
    return file.good();
}

void writeTextFile(const std::string& path, const std::string& data) {
    std::ofstream file(path, std::ios::trunc);
    file << data;
}

std::string readTextFile(const std::string& path) {
    std::ifstream file(path.c_str());
    file.seekg(0, std::ios::end);
    std::size_t size = file.tellg();
    std::string data;
    data.resize(size);

    file.seekg(0, std::ios::beg);
    file.read(data.data(), size);
    return data;
}

void runAll(TestRunner& runner) {
    runner.runTest("Parse open command", []() { testParseOpenCommand(); });
    runner.runTest("Parse quoted set command", []() { testParseQuotedSetCommand(); });
    runner.runTest("Parse help command", []() { testParseHelpCommand(); });
    runner.runTest("Execute open loads JSON", []() { testExecuteOpenLoadsJson(); });
    runner.runTest("Execute save writes JSON", []() { testExecuteSaveWritesJson(); });
    runner.runTest("Execute close closes file", []() { testExecuteCloseClosesFile(); });
}

// Ensures the open command parses command name and path
void testParseOpenCommand() {
    FileManager manager;
    JsonParser parser;
    Menu menu(manager, parser);
    Command cmd = menu.getCommand("open test_menu.json");
    require(cmd.commandEnum == CommandEnum::OPEN, "Expected OPEN command.");
    require(cmd.arg1 == "test_menu.json", "Expected file path argument.");
}

// Ensures quoted values preserve spaces in arguments
void testParseQuotedSetCommand() {
    FileManager manager;
    JsonParser parser;
    Menu menu(manager, parser);
    Command cmd = menu.getCommand("set Documents/Health/name \"New Value\"\n");
    require(cmd.commandEnum == CommandEnum::SET, "Expected SET command.");
    require(cmd.arg1 == "Documents/Health/name", "Expected path argument.");
    require(cmd.arg2 == "New Value", "Expected quoted value argument.");
}

// Ensures help parses without arguments
void testParseHelpCommand() {
    FileManager manager;
    JsonParser parser;
    Menu menu(manager, parser);
    Command cmd = menu.getCommand("help");
    require(cmd.commandEnum == CommandEnum::HELP, "Expected HELP command.");
}

// Verifies that executing open loads JSON into the parser
void testExecuteOpenLoadsJson() {
    FileManager manager;
    JsonParser parser;
    Menu menu(manager, parser);
    std::string filename = "menu_open.json";
    writeTextFile(filename, "{\"name\":\"Menu Test\"}");
    Command cmd{filename, "", CommandEnum::OPEN};
    menu.executeCommand(cmd);
    std::string data = parser.getData(0);
    require(!data.empty(), "Expected parser data after open.");
    requireContains(data, "\"name\"", "Expected JSON key in parser output.");
    requireContains(data, "\"Menu Test\"", "Expected JSON value in parser output.");
    manager.closeFile();
    std::remove(filename.data());
}

// Verifies that executing save writes parser data back to disk
void testExecuteSaveWritesJson() {
    FileManager manager;
    JsonParser parser;
    Menu menu(manager, parser);
    std::string filename = "menu_save.json";
    writeTextFile(filename, "{\"a\":1}");
    Command openCmd{filename, "", CommandEnum::OPEN};
    menu.executeCommand(openCmd);
    std::string value = "Added";
    parser.createValue("added", value);
    Command saveCmd{"", "", CommandEnum::SAVE};
    menu.executeCommand(saveCmd);
    manager.closeFile();
    std::string saved = readTextFile(filename);
    requireContains(saved, "\"added\"", "Expected new key in saved file.");
    requireContains(saved, "\"Added\"", "Expected new value in saved file.");
    std::remove(filename.data());
}

// Verifies that executing close closes the file stream
void testExecuteCloseClosesFile() {
    FileManager manager;
    JsonParser parser;
    Menu menu(manager, parser);
    std::string filename = "menu_close.json";
    writeTextFile(filename, "{\"a\":1}");
    Command openCmd{filename, "", CommandEnum::OPEN};
    menu.executeCommand(openCmd);
    Command closeCmd{"", "", CommandEnum::CLOSE};
    menu.executeCommand(closeCmd);
    std::string read = manager.readData();
    require(read.empty(), "Expected empty read after close.");
    std::remove(filename.c_str());
}



int main() {
    TestRunner runner;
    runAll(runner);
    return runner.printSummary() == 0 ? 0 : 1;
}
