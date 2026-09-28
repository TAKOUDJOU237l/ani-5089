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
    std::set<std::string> closure;
    std::vector<std::string> pending;

    while (iss >> name) {
        if (closure.insert(name).second) {
            pending.push_back(name);
        }
    }

    while (!pending.empty()) {
        std::string current = pending.back();
        pending.pop_back();

        auto it = needs.find(current);
        if (it != needs.end()) {
            for (const std::string &dep : it->second) {
                if (closure.insert(dep).second) {
                    pending.push_back(dep);
                }
            }
        }
    }

    std::map<std::string, int> neededBy;
    for (const std::string &module_name : closure) {
        neededBy[module_name] = 0;
    }
    for (const std::string &module_name : closure) {
        auto it = needs.find(module_name);
        if (it != needs.end()) {
            for (const std::string &dep : it->second) {
                neededBy[dep]++;
            }
        }
    }

    std::set<std::string> ready;
    for (const auto &entry : neededBy) {
        if (entry.second == 0) {
            ready.insert(entry.first);
        }
    }

    std::size_t printed = 0;
    while (!ready.empty()) {
        auto it = ready.begin();
        std::string current = *it;
        ready.erase(it);
        std::cout << current << "\n";
        ++printed;

        auto itNeeds = needs.find(current);
        if (itNeeds != needs.end()) {
            for (const std::string &dep : itNeeds->second) {
                neededBy[dep]--;
                if (neededBy[dep] == 0) {
                    ready.insert(dep);
                }
            }
        }
    }

    if (printed != closure.size()) {
        std::cout << "CYCLE\n";
    }

    return 0;
}