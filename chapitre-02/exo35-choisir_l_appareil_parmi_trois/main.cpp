#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

struct Device {
    std::string serial;
    std::string state;
    std::string model;
};

int main() {
    std::string line;

    std::getline(std::cin, line);
    int d = std::stoi(line);

    std::vector<Device> devices;
    for (int i = 0; i < d; ++i) {
        std::getline(std::cin, line);
        std::istringstream iss(line);
        Device dev;
        iss >> dev.serial >> dev.state >> dev.model;
        devices.push_back(dev);
    }

    std::getline(std::cin, line);
    std::string target = line;

    if (target != "-") {
        const Device *found = nullptr;
        for (const Device &dev : devices) {
            if (dev.serial == target) {
                found = &dev;
                break;
            }
        }

        if (found == nullptr) {
            std::cout << "ERREUR cible introuvable\n";
        } else if (found->state != "device") {
            std::cout << "ERREUR " << found->serial << " est " << found->state << "\n";
        } else {
            std::cout << found->serial << "\n";
        }
    } else {
        std::vector<std::string> ready;
        for (const Device &dev : devices) {
            if (dev.state == "device") {
                ready.push_back(dev.serial);
            }
        }

        if (ready.empty()) {
            std::cout << "ERREUR aucun appareil\n";
        } else if (ready.size() == 1) {
            std::cout << ready[0] << "\n";
        } else {
            std::sort(ready.begin(), ready.end());
            std::cout << "ERREUR plusieurs appareils\n";
            for (const std::string &serial : ready) {
                std::cout << serial << "\n";
            }
        }
    }

    return 0;
}
