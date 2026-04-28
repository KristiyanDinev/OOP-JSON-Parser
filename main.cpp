#include <iostream>
#include <string>
#include <fstream>

#include <menu.h>
#include <file.h>

int main() {
    Menu menu;
    menu.executeCommand(menu.waitForCommand());
    return 0;
}