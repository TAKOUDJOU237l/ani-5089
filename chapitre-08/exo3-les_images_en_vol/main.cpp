#include <iostream>
#include <string>

int main() {
    int vol = 0, n = 0;
    std::cin >> vol >> n;
    int enVol = 0, bloque = 0;
    for (int tour = 1; tour <= n; ++tour) {
        std::string cmd;
        std::cin >> cmd;
        bool attend = false;
        if (cmd == "SOUMET") {
            if (enVol >= vol) attend = true; else ++enVol;
        } else if (enVol > 0) {
            --enVol;
        }
        if (attend && bloque == 0) bloque = tour;
        std::cout << tour << ' ' << cmd << ' ' << enVol << ' '
                  << (attend ? "ATTEND" : "PASSE") << '\n';
    }
    std::cout << "BLOQUE " << bloque << '\n';
    return 0;
}
