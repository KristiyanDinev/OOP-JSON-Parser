#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#include "json.h"
#include "file.h"
#include "menu.h"

int main() {
    FileManager fileManager;
    JsonParser jsonParser;
    Menu menu = Menu(fileManager, jsonParser);
    while (true) {
        menu.executeCommand(menu.getCommand(menu.getInput()));
    }
    return 0;
}