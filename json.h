#include <vector>
#include <stack>
#include <string>
#include <sstream>
#include <iostream>
#include <cctype>

/*
 * key-value pair for JSON data.
 */
struct KeyValuePair {
    std::string key;
    std::string value;
};

/*
Path struct for working with paths in JSON.
*/
struct Path
{
    std::vector<std::string> Path;
    std::string value;
};

struct Bracket {
    unsigned int openInd;
    unsigned int closeInd;
    KeyValuePair keyValuePair;
    char bracket;
};

/*
 * Tracks whether a nested value should be recorded and which key it belongs to.
 */
struct TrackContext {
    const KeyValuePair* kvp;
    bool trackBracket;

    /*
     * Create a context that does not track any nested brackets.
     */
    TrackContext() : kvp(nullptr), trackBracket(false) {}

    /*
     * Create a context that tracks nested brackets for a specific key.
     */
    TrackContext(const KeyValuePair& trackedKvp, bool shouldTrack)
        : kvp(&trackedKvp), trackBracket(shouldTrack) {}

    /*
     * Return true when we should track and we have a valid key.
     */
    bool shouldTrack() const {
        return trackBracket && kvp != nullptr;
    }

    /*
     * Return the key/value reference used for tracking.
     */
    const KeyValuePair& keyValue() const {
        return *kvp;
    }
};


/*
 * Manages JSON data and objects.
 */
class JsonManager {
    public:
        /*
         * Initialize the manager with raw JSON text to parse and validate.
         */
        JsonManager(const std::string& jsonData) : data(jsonData) {}

        /*
         * Replace the current JSON text with new input.
         */
        void setData(const std::string& jsonData) {
            data = jsonData;
        }
        
        /*
         * Return the stored JSON text without formatting changes.
         */
        const std::string& prettyData() const {
            return data;
        }

        /*
        Valitate whether the Json data is valid or not. If it is not, then tell the user taht the file is not in a valid json format.
        and if you can point out where it is wrong. If it is valid, then tell the user that the file is in a valid json format.
        */
        /*
         * Validate the JSON text by parsing it from the start and reporting errors.
         */
        void validate(bool printErrors = false) {
            std::stringstream ss(data);

            ss >> std::ws;
            if (ss.peek() != '{') {
                if (printErrors) {
                    std::cout << "The file is not in a valid JSON format. Expected '{' at the beginning." << std::endl;
                }
                return;
            }
            keyValuePairs.clear();
            bracketStack = std::stack<Bracket>();
            if (!parseObject(ss, printErrors, TrackContext())) {
                return;
            }

            ss >> std::ws;
            if (ss.peek() != std::char_traits<char>::eof() && printErrors) {
                std::cout << "The file is not in a valid JSON format. Unexpected trailing characters." << std::endl;
            }
        }

        /*
         * Parse a JSON object and optionally track its raw substring for a key.
         */
        bool parseObject(std::stringstream& ss, bool printErrors, const TrackContext& ctx) {
            if (!startTrackingBracket(ss, '{', printErrors, ctx)) {
                return false;
            }
            return parseMembersTail(ss, printErrors, ctx);
        }

        /*
         * Parse a JSON array and optionally track its raw substring for a key.
         */
        bool parseArray(std::stringstream& ss, bool printErrors, const TrackContext& ctx) {
            if (!startTrackingBracket(ss, '[', printErrors, ctx)) {
                return false;
            }
            return parseElementsTail(ss, printErrors, ctx);
        }

        /*
         * Parse remaining object members using tail recursion to keep the logic flat.
         */
        bool parseMembersTail(std::stringstream& ss, bool printErrors, const TrackContext& ctx) {
            skipWhitespace(ss);
            if (ss.peek() == '}') {
                return closeBracket(ss, '}', printErrors, ctx);
            }

            KeyValuePair kvp;
            if (!readStringToken(ss, kvp.key, printErrors, "Expected string key.")) {
                return false;
            }
            if (!expectChar(ss, ':', printErrors, "Expected ':' after key.")) {
                return false;
            }
            if (!parseValue(ss, kvp, printErrors)) {
                return false;
            }
            return parseMembersSeparatorTail(ss, printErrors, ctx);
        }

        /*
         * Parse the separator after an object member and continue or close the object.
         */
        bool parseMembersSeparatorTail(std::stringstream& ss, bool printErrors, const TrackContext& ctx) {
            skipWhitespace(ss);
            if (ss.peek() == ',') {
                ss.get();
                return parseMembersTail(ss, printErrors, ctx);
            }
            if (ss.peek() == '}') {
                return closeBracket(ss, '}', printErrors, ctx);
            }
            return reportError(printErrors, "The file is not in a valid JSON format. Expected ',' or '}' in object.");
        }

        /*
         * Parse remaining array elements using tail recursion to avoid loops.
         */
        bool parseElementsTail(std::stringstream& ss, bool printErrors, const TrackContext& ctx) {
            skipWhitespace(ss);
            if (ss.peek() == ']') {
                return closeBracket(ss, ']', printErrors, ctx);
            }

            KeyValuePair kvp;
            if (!parseValue(ss, kvp, printErrors)) {
                return false;
            }
            return parseElementsSeparatorTail(ss, printErrors, ctx);
        }

        /*
         * Parse the separator after an array element and continue or close the array.
         */
        bool parseElementsSeparatorTail(std::stringstream& ss, bool printErrors, const TrackContext& ctx) {
            skipWhitespace(ss);
            if (ss.peek() == ',') {
                ss.get();
                return parseElementsTail(ss, printErrors, ctx);
            }
            if (ss.peek() == ']') {
                return closeBracket(ss, ']', printErrors, ctx);
            }
            return reportError(printErrors, "The file is not in a valid JSON format. Expected ',' or ']' in array.");
        }

        /*
         * Parse any JSON value and record it for the provided key when present.
         */
        bool parseValue(std::stringstream& ss, const KeyValuePair& kvp, bool printErrors) {
            skipWhitespace(ss);
            char nextChar = static_cast<char>(ss.peek());
            if (nextChar == '"') {
                std::string value;
                if (!readStringToken(ss, value, printErrors, "Expected string value.")) {
                    return false;
                }
                if (!kvp.key.empty()) {
                    keyValuePairs.push_back(KeyValuePair{kvp.key, value});
                }
                return true;
            }
            if (nextChar == '-' || std::isdigit(static_cast<unsigned char>(nextChar))) {
                std::string value;
                if (!readNumberToken(ss, value, printErrors)) {
                    return false;
                }
                if (!kvp.key.empty()) {
                    keyValuePairs.push_back(KeyValuePair{kvp.key, value});
                }
                return true;
            }
            if (nextChar == '{') {
                return parseObject(ss, printErrors, TrackContext(kvp, true));
            }
            if (nextChar == '[') {
                return parseArray(ss, printErrors, TrackContext(kvp, true));
            }
            return reportError(printErrors, "The file is not in a valid JSON format. Unexpected value.");
        }

        /*
         * Skip leading whitespace so parsing decisions can use peek safely.
         */
        void skipWhitespace(std::stringstream& ss) {
            ss >> std::ws;
        }

        /*
         * Consume a required character and report a custom error when missing.
         */
        bool expectChar(std::stringstream& ss, char expected, bool printErrors, const char* message) {
            skipWhitespace(ss);
            if (ss.peek() != expected) {
                return reportError(printErrors, message);
            }
            ss.ignore();
            return true;
        }

        /*
         * Read a quoted JSON string token into the output buffer.
         */
        bool readStringToken(std::stringstream& ss, std::string& out, bool printErrors, const char* message) {
            skipWhitespace(ss);
            if (ss.peek() != '"') {
                return reportError(printErrors, message);
            }
            ss.ignore();
            std::getline(ss, out, '"');
            return true;
        }

        /*
         * Read a numeric token, validating a basic JSON number shape.
         */
        bool readNumberToken(std::stringstream& ss, std::string& out, bool printErrors) {
            skipWhitespace(ss);
            if (!readNumberCharsTail(ss, out, true, true, true, false)) {
                return reportError(printErrors, "The file is not in a valid JSON format. Invalid number.");
            }
            return true;
        }

        /*
         * Recursively consume number characters and validate sign, dot, and exponent.
         */
        bool readNumberCharsTail(
            std::stringstream& ss,
            std::string& out,
            bool allowSign,
            bool allowDot,
            bool allowExp,
            bool hasDigit
        ) {
            char c = static_cast<char>(ss.peek());
            if (c == '-' || c == '+') {
                if (!allowSign) {
                    return hasDigit;
                }
                out.push_back(c);
                ss.ignore();
                return readNumberCharsTail(ss, out, false, allowDot, allowExp, hasDigit);
            }
            if (std::isdigit(static_cast<unsigned char>(c))) {
                out.push_back(c);
                ss.ignore();
                return readNumberCharsTail(ss, out, false, allowDot, allowExp, true);
            }
            if (c == '.' && allowDot) {
                out.push_back(c);
                ss.ignore();
                return readNumberCharsTail(ss, out, false, false, allowExp, hasDigit);
            }
            if ((c == 'e' || c == 'E') && allowExp) {
                out.push_back(c);
                ss.ignore();
                return readNumberCharsTail(ss, out, true, false, false, hasDigit);
            }
            return hasDigit;
        }

        /*
         * Consume an opening bracket and optionally start tracking its raw range.
         */
        bool startTrackingBracket(std::stringstream& ss, char expected, bool printErrors, const TrackContext& ctx) {
            skipWhitespace(ss);
            if (ss.peek() != expected) {
                return reportError(printErrors, "The file is not in a valid JSON format. Expected opening bracket.");
            }
            if (ctx.shouldTrack()) {
                Bracket bracket;
                bracket.bracket = expected;
                bracket.openInd = static_cast<unsigned int>(ss.tellg());
                bracket.keyValuePair = ctx.keyValue();
                bracketStack.push(bracket);
            }
            ss.ignore();
            return true;
        }

        /*
         * Consume a closing bracket and finalize any tracked raw substring.
         */
        bool closeBracket(std::stringstream& ss, char expected, bool printErrors, const TrackContext& ctx) {
            skipWhitespace(ss);
            if (ss.peek() != expected) {
                return reportError(printErrors, "The file is not in a valid JSON format. Expected closing bracket.");
            }
            if (ctx.shouldTrack()) {
                if (bracketStack.empty()) {
                    return reportError(printErrors, "The file is not in a valid JSON format. Unexpected closing bracket.");
                }
                Bracket topBracket = bracketStack.top();
                if ((expected == '}' && topBracket.bracket != '{') || (expected == ']' && topBracket.bracket != '[')) {
                    return reportError(printErrors, "The file is not in a valid JSON format. Mismatched closing bracket.");
                }
                ss.ignore();
                topBracket.closeInd = static_cast<unsigned int>(ss.tellg());
                bracketStack.pop();
                topBracket.keyValuePair.value = data.substr(topBracket.openInd, topBracket.closeInd - topBracket.openInd);
                if (!topBracket.keyValuePair.key.empty()) {
                    keyValuePairs.push_back(topBracket.keyValuePair);
                }
                return true;
            }
            ss.ignore();
            return true;
        }

        /*
         * Emit an error message if requested and signal failure to the caller.
         */
        bool reportError(bool printErrors, const char* message) {
            if (printErrors) {
                std::cout << message << std::endl;
            }
            return false;
        }

    private:
        std::string data;
        std::vector<KeyValuePair> keyValuePairs;
        std::stack<Bracket> bracketStack;
};