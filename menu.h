#include <sstream>

std::string extractNextArgument(const std::string& input, unsigned int& position) {
    while (position < input.length() && input[position] == ' ') {
        position++;
    }

    if (position >= input.length()) {
        return "";
    }

    std::string result;

    if (input[position] == '"') {
        position++;
        while (position < input.length() && input[position] != '"') {
            result += input[position];
            position++;
        }

        if (position < input.length()) {
            position++;
        }

        return result;
    }

    while (position < input.length() && input[position] != ' ') {
        result += input[position];
        position++;
    }

    return result;
}


/* Storing all of the commands for better
classification and work with them.
*/
enum CommandEnum {
    OPEN,
    CLOSE,
    SAVE,
    SAVEAS,
    HELP,
    EXIT,
    VALIDATE,
    PRINT,
    SEARCH,
    SET,
    CREATE,
    DELETE,
    MOVE
};

/* The struck Command, which will be used to
pass the whole command into functions and since
the maximum number of arguments for a single command
is 2. We have arg1 and arg2. We use struct instead of
classes, because struck has all we need and classes
only have more functionality and features we don't need.
*/
struct Command
{
    CommandEnum commandEnum;
    std::string arg1;
    std::string arg2;
};

/* This is our Menu class, which will be used for
implimenting the menu feature.
*/
class Menu {
    public:
        void printHelp() {
            std::cout << "\n--------\nФайлови Команди" 
            << "\nopen [/path/to/file] - Отваря дадения от потребителя файл в апликацията за обработка. Ако такъв файл не съществува, тогава програмата създава нов JSON файл с името file.json на това място за обработка и ще даде грешка, ако не може да го създаде."
            << "\nclose - Затваря отворения файл от апликацията без да го запазва."
            << "\nsave - Запазва отворения файл на диска и държи файла отворен."
            << "\nsaveas - Запазва отворения файл на като нов файл."
            << "\nhelp - Показва помощното меню с всички команди и тяхната информация."
            << "\nexit - Излиза от програмата. Ако има отворен файл, то той няма да се запази и цялата модифицирана информация се губи, ако има такава."
            << "\n\n JSON Команди"
            << "\nvalidate - Дава обратна връзка на потребителя дали JSON файла е написан в правилен формат или има синтактична грешка."
            << "\nprint - Изпечатва цялата информация от JSON файла на по-разчетим начин."
            << "\nsearch <key> - Извежда масив от всички стойности, които имат подадения ключ."
            << "\nset <path> <string> - Обновяване на даден път с нова стойност. Ако този път не съществува, то ще има грешка."
            << "\ncreate <path> <string> - Създава нов път с нова стойност. Ако пътя вече съществува, тогава ще има грешка."
            << "\ndelete <path> - Изтриване на път и съответно нейната стойност."
            << "\nmove <from path> <to path> - Елементите с път from path ще се преместят и ще имат нов път to path.\n--------\n";
        }

        Command getCommand() {
            std::string input;
            std::getline(std::cin, input);
            Command cmd;

            /*
            За по-опимизирана работа, ще ни трябва да знаем къде свършва
            командата и от къде започват аргументити, за да не го прави два пъти едно и също.
            */ 
            unsigned int commandStopInd = 0;
            while (commandStopInd < input.length() && input[commandStopInd] != ' ') {
                commandStopInd++;
            }

            getCommandEnumFromInput(input, cmd, commandStopInd);
            if (cmd.commandEnum == CommandEnum::HELP ||
                cmd.commandEnum == CommandEnum::EXIT ||
                cmd.commandEnum == CommandEnum::VALIDATE ||
                cmd.commandEnum == CommandEnum::PRINT ||
                cmd.commandEnum == CommandEnum::SAVE ||
                cmd.commandEnum == CommandEnum::CLOSE) {
                return cmd;
            }

            parseArguments(input, cmd, commandStopInd);
            return cmd;
        }

        Command waitForCommand() {
            Command cmd;
            while (true) {
                cmd = getCommand();
                return cmd;
            }
        }

        /* Този код menu.executeCommand(menu.waitForCommand()); се изпълнява
        в главната функция.
        */
        void executeCommand(Command cmd) {
            switch (cmd.commandEnum) {
                case CommandEnum::OPEN:
                    break;
                case CommandEnum::CLOSE:
                    break;
                case CommandEnum::SAVE:
                    break;
                case CommandEnum::SAVEAS:
                    break;
                case CommandEnum::HELP:
                    printHelp();
                    break;
                case CommandEnum::EXIT:
                    break;
                case CommandEnum::VALIDATE:
                    break;
                case CommandEnum::PRINT:
                    break;
                case CommandEnum::SEARCH:
                    break;
                case CommandEnum::SET:
                    break;
                case CommandEnum::CREATE:
                    break;
                case CommandEnum::DELETE:
                    break;
                case CommandEnum::MOVE:
                    break;

                default:
                    printHelp();
                    break;
            }
        }

    private:
        std::string toLowerCase(std::string& input) {
            char* str = input.data();
            for (int i = 0; i < input.length(); i++) {
                str[i] = std::tolower(str[i]);
            }
            return input;
        }

        void getCommandEnumFromInput(const std::string& input, Command& cmd, unsigned int commandStopInd) {
            std::string commandName;
            if (commandStopInd == 0) {
                cmd.commandEnum = CommandEnum::HELP;
                return;
            } else {
                commandName = toLowerCase(input.substr(0, commandStopInd));
            }

            if (commandName == "open") cmd.commandEnum = CommandEnum::OPEN;
            if (commandName == "close") cmd.commandEnum = CommandEnum::CLOSE;
            if (commandName == "save") cmd.commandEnum = CommandEnum::SAVE;
            if (commandName == "saveas") cmd.commandEnum = CommandEnum::SAVEAS;
            if (commandName == "help") cmd.commandEnum = CommandEnum::HELP;
            if (commandName == "exit") cmd.commandEnum = CommandEnum::EXIT;
            if (commandName == "validate") cmd.commandEnum = CommandEnum::VALIDATE;
            if (commandName == "print") cmd.commandEnum = CommandEnum::PRINT;
            if (commandName == "search") cmd.commandEnum = CommandEnum::SEARCH;
            if (commandName == "set") cmd.commandEnum = CommandEnum::SET;
            if (commandName == "create") cmd.commandEnum = CommandEnum::CREATE;
            if (commandName == "delete") cmd.commandEnum = CommandEnum::DELETE;
            if (commandName == "move") cmd.commandEnum = CommandEnum::MOVE;

            cmd.commandEnum = CommandEnum::HELP;
        }

        void parseArguments(const std::string& input, Command& cmd, unsigned int commandStopInd) {
            std::stringstream ss(input.substr(commandStopInd));
            std::string word;

            ss >> std::ws;

            if (ss.peek() == '"') {
                ss.ignore();
                std::getline(ss, cmd.arg1, '"');
            } else {
                ss >> word;
                cmd.arg1 = word;
            }

            ss >> std::ws;

            if (ss.peek() == '"') {
                ss.ignore();
                std::getline(ss, cmd.arg2, '"');
            } else {
                ss >> word;
                cmd.arg2 = word;
            }
        }
};