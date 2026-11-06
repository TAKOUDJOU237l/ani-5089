#include <iostream>
#include <string>

int main() {
    int images = 0, n = 0;
    std::cin >> images >> n;
    int suivant = 0, prets = 0;
    bool emprunte = false;
    for (int tour = 1; tour <= n; ++tour) {
        std::string cmd;
        std::cin >> cmd;
        std::cout << tour << ' ' << cmd << ' ';
        if (cmd == "EMPRUNTE") {
            if (emprunte) {
                std::cout << "REFUSE\n";
            } else {
                emprunte = true;
                ++prets;
                std::cout << suivant << '\n';
            }
        } else {
            if (!emprunte) {
                std::cout << "REFUSE\n";
            } else {
                emprunte = false;
                std::cout << suivant << '\n';
                suivant = (suivant + 1) % images;
            }
        }
    }
    std::cout << "PRETS " << prets << '\n';
    return 0;
}
