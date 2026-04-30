
// Enumerates supported console commands.
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

// Holds a parsed command and up to two arguments.
struct Command
{
    std::string arg1;
    std::string arg2;
    CommandEnum commandEnum;
};

// Coordinates user input parsing and command execution.
class Menu {
    public:
        // Binds to the active file manager and JSON parser.
        Menu(FileManager& fileManager, JsonParser& jsonParser) :
         fileManager(fileManager), jsonParser(jsonParser) {}

        // Prints a full command reference to the console.
        void printHelp() {
            std::cout << "\n--------\n File Commands"
            << "\n\nopen [/path/to/file] - Opens the file selected by the user in the application for processing. If the file does not exist, the program creates a new JSON file named file.json at that location for processing, and it will report an error if it cannot create it."
            << "\n\nclose - Closes the opened file in the application without saving it."
            << "\n\nsave - Saves the opened file to disk and keeps it open."
            << "\n\nsaveas - Saves the opened file as a new file."
            << "\n\nhelp - Shows the help menu with all commands and their information."
            << "\n\nexit - Exits the program. If a file is open, it will not be saved and all modified information will be lost, if any exists."
            << "\n\n JSON Commands"
            << "\n\nvalidate - Provides feedback to the user about whether the JSON file is written in a valid format or contains a syntax error."
            << "\n\nprint - Prints all information from the JSON file in a more readable format."
            << "\n\nsearch <key> - Outputs an array of all values that have the provided key."
            << "\n\nset <path> <string> - Updates the given path with a new value. If the path does not exist, an error will occur."
            << "\n\ncreate <path> <string> - Creates a new path with a new value. If the path already exists, an error will occur."
            << "\n\ndelete <path> - Deletes a path and its corresponding value."
            << "\n\nmove <from path> <to path> - Elements at the from path will be moved and assigned the new to path.\n--------\n";
        }

        // Reads a line and returns the parsed command.
        std::string getInput() {
            std::string input;
            std::getline(std::cin, input);
            return input;
        }

        Command getCommand(const std::string& input) {
            Command cmd;

            // Locate the command boundary to avoid reparsing arguments.
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

        // Executes the command and dispatches to the correct handler.
        void executeCommand(Command cmd) {
            switch (cmd.commandEnum) {
                case CommandEnum::OPEN:
                    fileManager.openFile(cmd.arg1);
                    jsonParser.setJsonData(fileManager.readData());
                    break;
                case CommandEnum::CLOSE:
                    fileManager.closeFile();
                    break;
                case CommandEnum::SAVE:
                    fileManager.saveData(jsonParser.getData(0));
                    break;
                case CommandEnum::SAVEAS:
                    fileManager.saveAs(cmd.arg1, jsonParser.getData(0));
                    break;
                case CommandEnum::HELP:
                    printHelp();
                    break;
                case CommandEnum::EXIT:
                    exit(0);
                    break;
                case CommandEnum::VALIDATE:
                    jsonParser.validate(true);
                    break;
                case CommandEnum::PRINT:
                    jsonParser.print();
                    break;
                case CommandEnum::SEARCH:
                    jsonParser.search(cmd.arg1);
                    break;
                case CommandEnum::SET:
                    jsonParser.setPathValue(cmd.arg1, cmd.arg2);
                    break;
                case CommandEnum::CREATE:
                    jsonParser.createValue(cmd.arg1, cmd.arg2);
                    break;
                case CommandEnum::DELETE:
                    jsonParser.deleteValue(cmd.arg1);
                    break;
                case CommandEnum::MOVE:
                    jsonParser.moveValue(cmd.arg1, cmd.arg2);
                    break;

                default:
                    printHelp();
                    break;
            }
        }

    private:
        FileManager& fileManager;
        JsonParser& jsonParser;

        // Normalizes the command name for matching.
        std::string toLowerCase(char* input, int size) {
            for (int i = 0; i < size; i++) {
                input[i] = std::tolower(input[i]);
            }
            return std::string(input, size);
        }

        // Maps the command text to its enum value.
        void getCommandEnumFromInput(const std::string& input, Command& cmd, unsigned int commandStopInd) {
            std::string commandName;
            if (commandStopInd == 0) {
                cmd.commandEnum = CommandEnum::HELP;
                return;
            } else {
                std::string substr = input.substr(0, commandStopInd);
                commandName = toLowerCase(substr.data(), substr.length());
            }

            if (commandName == "open") cmd.commandEnum = CommandEnum::OPEN;
            else if (commandName == "close") cmd.commandEnum = CommandEnum::CLOSE;
            else if (commandName == "save") cmd.commandEnum = CommandEnum::SAVE;
            else if (commandName == "saveas") cmd.commandEnum = CommandEnum::SAVEAS;
            else if (commandName == "help") cmd.commandEnum = CommandEnum::HELP;
            else if (commandName == "exit") cmd.commandEnum = CommandEnum::EXIT;
            else if (commandName == "validate") cmd.commandEnum = CommandEnum::VALIDATE;
            else if (commandName == "print") cmd.commandEnum = CommandEnum::PRINT;
            else if (commandName == "search") cmd.commandEnum = CommandEnum::SEARCH;
            else if (commandName == "set") cmd.commandEnum = CommandEnum::SET;
            else if (commandName == "create") cmd.commandEnum = CommandEnum::CREATE;
            else if (commandName == "delete") cmd.commandEnum = CommandEnum::DELETE;
            else if (commandName == "move") cmd.commandEnum = CommandEnum::MOVE;
            else cmd.commandEnum = CommandEnum::HELP;
        }

        // Parses up to two arguments, supporting quoted strings.
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
            int ind = ss.tellg();
            ss >> std::ws;

            if (ss.tellg() == ind) {
                return;
            }
        
            if (ss.peek() == '"') {
                ss.ignore();
                std::getline(ss, cmd.arg2, '"');
            } else {
                ss >> word;
                cmd.arg2 = word;
            }
        }
};