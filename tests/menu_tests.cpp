#include <functional>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "../json.h"
#include "../file.h"
#include "../menu.h"

// Records a single test result with a name and optional failure detail.
struct TestResult {
    std::string name;
    std::string detail;
    bool passed;
};


// Fails the test when the condition is false.
void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
 }

// Fails the test when the expected substring is missing.
void requireContains(const std::string& text, 
    const std::string& expected, const std::string& message) {
    if (text.find(expected) == std::string::npos) {
        throw std::runtime_error(message);
    }
}

// Runs tests and prints progress plus a final summary.
class TestRunner {
public:
    // Executes a single test and records its result.
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

    // Prints a summary and returns the number of failed tests.
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


// Checks whether a file can be opened for reading.
bool fileExists(const std::string& path) {
    std::ifstream file(path);
    return file.good();
}

// Writes text to a file, overwriting existing contents.
void writeTextFile(const std::string& path, const std::string& data) {
    std::ofstream file(path, std::ios::trunc);
    file << data;
}

// Reads the entire contents of a file into a string.
std::string readTextFile(const std::string& path) {
    std::ifstream file(path.c_str());
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    std::string data;
    data.resize(size);

    file.seekg(0, std::ios::beg);
    file.read(data.data(), size);
    return data;
}

// Groups Menu tests into an OOP-style suite.
class MenuTests {
public:
    // Registers and runs all Menu tests.
    void runAll(TestRunner& runner) {
        runner.runTest("Parse open command", [this]() { testParseOpenCommand(); });
        runner.runTest("Parse quoted set command", [this]() { testParseQuotedSetCommand(); });
        runner.runTest("Parse help command", [this]() { testParseHelpCommand(); });
        runner.runTest("Execute open loads JSON", [this]() { testExecuteOpenLoadsJson(); });
        runner.runTest("Execute save writes JSON", [this]() { testExecuteSaveWritesJson(); });
        runner.runTest("Execute close closes file", [this]() { testExecuteCloseClosesFile(); });
    }

private:
    // Ensures the open command parses command name and path.
    void testParseOpenCommand() const {
        FileManager manager;
        JsonParser parser;
        Menu menu(manager, parser);
        Command cmd = menu.getCommand("open test_menu.json");
        require(cmd.commandEnum == CommandEnum::OPEN, "Expected OPEN command.");
        require(cmd.arg1 == "test_menu.json", "Expected file path argument.");
    }

    // Ensures quoted values preserve spaces in arguments.
    void testParseQuotedSetCommand() const {
        FileManager manager;
        JsonParser parser;
        Menu menu(manager, parser);
        Command cmd = menu.getCommand("set Documents/Health/name \"New Value\"\n");
        require(cmd.commandEnum == CommandEnum::SET, "Expected SET command.");
        require(cmd.arg1 == "Documents/Health/name", "Expected path argument.");
        require(cmd.arg2 == "New Value", "Expected quoted value argument.");
    }

    // Ensures help parses without arguments.
    void testParseHelpCommand() const {
        FileManager manager;
        JsonParser parser;
        Menu menu(manager, parser);
        Command cmd = menu.getCommand("help");
        require(cmd.commandEnum == CommandEnum::HELP, "Expected HELP command.");
    }

    // Verifies that executing open loads JSON into the parser.
    void testExecuteOpenLoadsJson() const {
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

    // Verifies that executing save writes parser data back to disk.
    void testExecuteSaveWritesJson() const {
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

    // Verifies that executing close closes the file stream.
    void testExecuteCloseClosesFile() const {
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
};


int main() {
    TestRunner runner;
    MenuTests tests;
    tests.runAll(runner);
    return runner.printSummary() == 0 ? 0 : 1;
}
