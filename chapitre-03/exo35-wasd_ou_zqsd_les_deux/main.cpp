#include <iostream>
#include <sstream>
#include <string>

int main() {
    std::string ligne;
    std::getline(std::cin, ligne);
    int n = std::stoi(ligne);

    for (int i = 0; i < n; ++i) {
        std::getline(std::cin, ligne);
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
        std::cout << deplacement << " " << cote << "\n";
    }

    return 0;
}
