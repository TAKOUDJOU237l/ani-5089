#include <iostream>
#include <sstream>
#include <string>

int main() {
    std::string line;

    std::getline(std::cin, line);
    long long budget = std::stoll(line);

    std::getline(std::cin, line);
    int s = std::stoi(line);

    int trompeCount = 0;

    for (int i = 0; i < s; ++i) {
        std::getline(std::cin, line);
        std::istringstream iss(line);

        std::string name;
        long long debug, release;
        iss >> name >> debug >> release;

        long long factor = (debug + release / 2) / release;
        bool tient = (release <= budget);

        std::cout << name << " " << factor << " " << (tient ? "TIENT" : "DEPASSE") << "\n";

        if (debug > budget && release <= budget) {
            ++trompeCount;
        }
    }

    std::cout << "TROMPE " << trompeCount << "\n";

    return 0;
}
