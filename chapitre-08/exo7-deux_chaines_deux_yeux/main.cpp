#include <cstdlib>
#include <iostream>
#include <string>

struct Anneau {
    int suivant = 0;
    bool emprunte = false;
    int rendus = 0;
};

int main() {
    int images = 0, n = 0;
    std::cin >> images >> n;
    Anneau yeux[2];
    int desync = 0, ecart = 0;
    for (int tour = 1; tour <= n; ++tour) {
        std::string oeil, cmd;
        std::cin >> oeil >> cmd;
        Anneau &a = yeux[oeil == "G" ? 0 : 1];
        std::cout << tour << ' ' << oeil << ' ' << cmd << ' ';
        if (cmd == "EMPRUNTE") {
            if (a.emprunte) std::cout << "REFUSE\n";
            else { a.emprunte = true; std::cout << a.suivant << '\n'; }
        } else {
            if (!a.emprunte) std::cout << "REFUSE\n";
            else {
                a.emprunte = false;
                ++a.rendus;
                std::cout << a.suivant << '\n';
                a.suivant = (a.suivant + 1) % images;
            }
        }
        ecart = std::abs(yeux[0].rendus - yeux[1].rendus);
        if (ecart > 1 && desync == 0) desync = tour;
    }
    std::cout << "DECALAGE " << ecart << '\n';
    std::cout << "DESYNC " << desync << '\n';
    return 0;
}
