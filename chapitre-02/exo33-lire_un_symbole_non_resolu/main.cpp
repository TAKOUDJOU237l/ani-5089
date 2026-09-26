#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <set>

int main() {
    std::string line;

    std::getline(std::cin, line);
    int p = std::stoi(line);

    std::vector<std::pair<std::string, std::string>> prefixes; 
    for (int i = 0; i < p; ++i) {
        std::getline(std::cin, line);
        std::istringstream iss(line);
        std::string prefix, moduleName;
        iss >> prefix >> moduleName;
        prefixes.push_back({prefix, moduleName});
    }

    std::getline(std::cin, line);
    int l = std::stoi(line);

    const std::string marker = "undefined reference to '";
    std::set<std::string> modules;
    int unknownCount = 0;

    for (int i = 0; i < l; ++i) {
        std::getline(std::cin, line);

        size_t pos = line.find(marker);
        if (pos == std::string::npos) {
            continue;
        }

        size_t start = pos + marker.size();
        size_t end = line.find('\'', start);
        if (end == std::string::npos) {
            continue;
        }

        std::string symbol = line.substr(start, end - start);

        std::string bestModule;
        size_t bestLength = 0;
        bool found = false;

        for (const auto &entry : prefixes) {
            const std::string &prefix = entry.first;
            if (symbol.size() >= prefix.size() &&
                symbol.compare(0, prefix.size(), prefix) == 0) {
                if (prefix.size() > bestLength) {
                    bestLength = prefix.size();
                    bestModule = entry.second;
                    found = true;
                }
            }
        }

        if (found) {
            modules.insert(bestModule);
        } else {
            ++unknownCount;
        }
    }

    for (const std::string &moduleName : modules) {
        std::cout << moduleName << "\n";
    }

    if (unknownCount > 0) {
        std::cout << "INCONNU " << unknownCount << "\n";
    }

    return 0;
}
