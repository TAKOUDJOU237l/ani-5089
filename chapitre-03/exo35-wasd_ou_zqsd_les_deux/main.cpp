#include <iostream>
#include <sstream>
#include <string>

int main() {
    long long n = 0;
    if (!(std::cin >> n)) {
        return 0;
    }

    std::string ligne;
    std::getline(std::cin, ligne);

    for (long long i = 0; i < n; ++i) {
        if (!std::getline(std::cin, ligne)) {
            break;
        }
        std::istringstream flux(ligne);

        bool avance = false;
        bool recule = false;
        bool gauche = false;
        bool droite = false;

        std::string touche;
        while (flux >> touche) {
            if (touche == "W" || touche == "Z") {
                avance = true;
            } else if (touche == "S") {
                recule = true;
            } else if (touche == "A" || touche == "Q") {
                gauche = true;
            } else if (touche == "D") {
                droite = true;
            }
        }

        int deplacement = (avance ? 1 : 0) - (recule ? 1 : 0);
        int cote = (droite ? 1 : 0) - (gauche ? 1 : 0);
        std::cout << deplacement << " " << cote << std::endl;
    }

    return 0;
}