#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>

std::string trim(const std::string &s) {
    size_t start = s.find_first_not_of(" \t");
    if (start == std::string::npos) {
        return "";
    }
    size_t end = s.find_last_not_of(" \t");
    return s.substr(start, end - start + 1);
}

std::vector<std::string> splitOnAnd(const std::string &condition) {
    std::vector<std::string> parts;
    size_t pos = 0;
    while (true) {
        size_t idx = condition.find("&&", pos);
        if (idx == std::string::npos) {
            parts.push_back(condition.substr(pos));
            break;
        }
        parts.push_back(condition.substr(pos, idx - pos));
        pos = idx + 2;
    }
    return parts;
}

int main() {
    std::string line;

    std::getline(std::cin, line);
    int v = std::stoi(line);

    std::map<std::string, std::string> machine;
    for (int i = 0; i < v; ++i) {
        std::getline(std::cin, line);
        size_t eq = line.find('=');
        std::string key = trim(line.substr(0, eq));
        std::string value = trim(line.substr(eq + 1));
        machine[key] = value;
    }

    std::getline(std::cin, line);
    int f = std::stoi(line);

    for (int i = 0; i < f; ++i) {
        std::getline(std::cin, line);

        bool result = true;
        for (const std::string &rawTerm : splitOnAnd(line)) {
            std::string term = trim(rawTerm);

            bool negate = false;
            if (!term.empty() && term[0] == '!') {
                negate = true;
                term = trim(term.substr(1));
            }

            size_t eq = term.find('=');
            std::string key = term.substr(0, eq);
            std::string value = term.substr(eq + 1);

            bool termValue;
            auto it = machine.find(key);
            if (it == machine.end()) {
                termValue = false;
            } else {
                termValue = (it->second == value);
            }

            if (negate) {
                termValue = !termValue;
            }

            if (!termValue) {
                result = false;
            }
        }

        std::cout << (result ? "OUI" : "NON") << "\n";
    }

    return 0;
}
