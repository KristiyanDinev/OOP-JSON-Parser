#pragma once

#include <fstream>
#include <iostream>
#include <string>

#include "file.hpp"

// Manages file lifecycle and persistence for JSON data.
// Opens a file for read/write; falls back to default on failure.
bool FileManager::openFile(std::string& filename) {
    file.clear();
    createFile(filename);
    file.open(filename, std::ios::in | std::ios::out);
    if (!file.is_open()) {
        file.clear();
        createFile(defaultFileName);
        file.open(defaultFileName, std::ios::in | std::ios::out);
        std::cout << "Can't open/create the file: " << filename << std::endl;
        if (!file.is_open()) {
            file.clear();
            std::cout << "Can't open/create the file: " << defaultFileName << std::endl;
            return false;
        }
        filename = defaultFileName;
    }
    currentFileName = filename;
    std::cout << "The file is open/create: " << filename << std::endl;
    return true;
}

// Ensures a file exists by opening it in append mode.
void FileManager::createFile(const std::string& name) const {
    std::fstream created(name, std::ios::app);
}

// Closes the current file and resets its state.
void FileManager::closeFile() {
    if (file.is_open()) {
        file.close();
        std::cout << "The file is closed." << std::endl;
    }
    file.clear();
}

// Truncates and writes the provided JSON data to the current file.
void FileManager::saveData(const std::string& data) {
    if (file.is_open()) {
        file.close();
        file.clear();
        file.open(currentFileName, std::ios::in | std::ios::out | std::ios::trunc);
        if (!file.is_open()) {
            std::cout << "The file is not open. Data cannot be written." << std::endl;
            return;
        }
        file << data << std::endl;
        file.flush();
        std::cout << "The data has been written to the file." << std::endl;
    } else {
        std::cout << "The file is not open. Data cannot be written." << std::endl;
    }
}

// Reads the entire file content into a string.
std::string FileManager::readData() {
    if (!file.is_open()) {
        return "";
    }
    file.clear();
    file.seekg(0, std::ios::end);
    std::size_t size = file.tellg();
    std::string data;
    data.resize(size);

    file.seekg(0, std::ios::beg);
    file.read(data.data(), size);
    return data;
}

// Saves data to a new file path and keeps it open.
void FileManager::saveAs(std::string& newFilename, const std::string& data) {
    if (file.is_open()) {
        file.close();
    }
    if (!openFile(newFilename)) {
        return;
    }
    saveData(data);
}

// Stream for the currently opened file.
// Default file name used when opening fails.
// Tracks the active file path for save operations.
