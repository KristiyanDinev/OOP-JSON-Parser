#include <functional>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "../file.cpp"

// The results from the test
struct TestResult {
    std::string name;
    std::string detail;
    bool passed;
};


// Fails the test when the condition is false
void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

// Fails the test when the expected substring is missing
void requireContains(const std::string& text, const std::string& expected, const std::string& message) {
    if (text.find(expected) == std::string::npos) {
        throw std::runtime_error(message);
    }
}

// Running tests and saving their results
class TestRunner {
public:
    // Executes a test and records it
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

// Checks whether a file can be opened for reading
bool fileExists(const std::string& path) {
    std::ifstream file(path);
    return file.good();
}

// Writes text to a file overwriting existing contents
void writeTextFile(const std::string& path, const std::string& data) {
    std::ofstream file(path, std::ios::trunc);
    file << data;
}

// Get whole file context
std::string readTextFile(const std::string& path) {
    std::ifstream file(path);
    file.seekg(0, std::ios::end);
    std::size_t size = file.tellg();
    std::string data;
    data.resize(size);

    file.seekg(0, std::ios::beg);
    file.read(data.data(), size);
    return data;
}

// Verifies that openFile creates missing files
void testOpenCreatesFile() {
    FileManager manager;
    std::string filename = "test_open_create.json";
    const char* fileNameData = filename.data();
    std::remove(fileNameData);
    bool opened = manager.openFile(filename);
    require(opened, "Expected file to open successfully.");
    require(fileExists(filename), "Expected file to exist after open.");
    manager.closeFile();
    std::remove(fileNameData);
}

// Verifies that saveData persists content and readData returns it
void testSaveAndReadData() {
    FileManager manager;
    std::string filename = "test_save_read.json";
    const char* fileNameData = filename.data();
    std::remove(fileNameData);
    manager.openFile(filename);
    std::string data = "{\"a\":1}";
    manager.saveData(data);
    std::string read = manager.readData();
    requireContains(read, data, "Expected saved JSON content in file.");
    manager.closeFile();
    std::remove(fileNameData);
}

// Verifies that saveAs writes to a new file and updates the file handle
void testSaveAsWritesNewFile() {
    FileManager manager;
    std::string filename = "test_saveas_from.json";
    std::string newFilename = "test_saveas_to.json";

    const char* fileNameData = filename.data();
    const char* newFileNameData = newFilename.data();

    std::remove(fileNameData);
    std::remove(newFileNameData);
    manager.openFile(filename);
    std::string data = "{\"k\":\"v\"}";
    manager.saveAs(newFilename, data);
    require(fileExists(newFilename), "Expected saveAs file to exist.");
    std::string read = readTextFile(newFilename);
    requireContains(read, data, "Expected saveAs data in new file.");
    manager.closeFile();
    std::remove(fileNameData);
    std::remove(newFileNameData);
}

// Ensures readData returns empty when the file is closed
void testCloseResetsRead() {
    FileManager manager;
    std::string filename = "test_close_read.json";
    const char* fileNameData = filename.data();

    std::remove(fileNameData);
    manager.openFile(filename);
    manager.closeFile();
    std::string read = manager.readData();
    require(read.empty(), "Expected empty read after close.");
    std::remove(fileNameData);
}

// Verifies fallback to default file when opening an invalid path
void testOpenInvalidPathFallsBack() {
    FileManager manager;
    std::string filename = "nonexistent_dir_for_tests_9f8e7d\\bad.json";
    bool opened = manager.openFile(filename);
    require(opened, "Expected fallback open to succeed.");
    require(filename == "file.json", "Expected fallback file name to be file.json.");
    require(fileExists("file.json"), "Expected default file to exist after fallback.");
    manager.closeFile();
    std::remove("file.json");
}

// Run all tests
void runAll(TestRunner& runner) {
    runner.runTest("Open creates file", []() { testOpenCreatesFile(); });
    runner.runTest("Save and read data", []() { testSaveAndReadData(); });
    runner.runTest("SaveAs writes new file", []() { testSaveAsWritesNewFile(); });
    runner.runTest("Close resets read", []() { testCloseResetsRead(); });
    runner.runTest("Open invalid path falls back", []() { testOpenInvalidPathFallsBack(); });
}

int main() {
    TestRunner runner;
    runAll(runner);
    return runner.printSummary() == 0 ? 0 : 1;
}
