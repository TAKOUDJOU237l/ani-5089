#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>

int main() {
    std::string line;

    std::getline(std::cin, line);
    int n = std::stoi(line);

    std::map<std::string, std::vector<std::string>> needs;

    for (int i = 0; i < n; ++i) {
        std::getline(std::cin, line);
        std::istringstream iss(line);
        std::string name;
        iss >> name;

        std::vector<std::string> deps;
        std::string dep;
        while (iss >> dep) {
            deps.push_back(dep);
        }
        needs[name] = deps;
    }

    std::getline(std::cin, line); 
    std::getline(std::cin, line); 

    std::istringstream iss(line);
    std::string name;
    std::set<std::string> reached;
    std::vector<std::string> pending;

    while (iss >> name) {
        if (reached.insert(name).second) {
            pending.push_back(name);
        }
    }

    while (!pending.empty()) {
        std::string current = pending.back();
        pending.pop_back();

        auto it = needs.find(current);
        if (it != needs.end()) {
            for (const std::string &dep : it->second) {
                if (reached.insert(dep).second) {
                    pending.push_back(dep);
                }
            }
        }
    }

    for (const std::string &module_name : reached) {
        std::cout << module_name << "\n";
    }

    return 0;
}
