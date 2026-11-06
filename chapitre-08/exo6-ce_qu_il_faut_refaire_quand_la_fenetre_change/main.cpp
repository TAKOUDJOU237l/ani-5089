#include <iostream>
#include <sstream>
#include <string>

int main() {
    int n = 0;
    std::string ligne;
    std::getline(std::cin, ligne);
    n = std::stoi(ligne);
    int survivants = 0;
    for (int i = 0; i < n; ++i) {
        std::getline(std::cin, ligne);
        std::istringstream flux(ligne);
        std::string nom, mot;
        flux >> nom;
        int posDetruit = -1, posRecree = -1;
        bool reenregistre = false;
        int pos = 0;
        while (flux >> mot) {
            if (mot == "DETRUIT" && posDetruit < 0) posDetruit = pos;
            else if (mot == "RECREE" && posRecree < 0) posRecree = pos;
            else if (mot == "REENREGISTRE") reenregistre = true;
            ++pos;
        }
        const char *verdict = "SURVIT";
        if (posDetruit >= 0 && posRecree >= 0 && posRecree < posDetruit) verdict = "MAUVAIS ORDRE";
        else if (posDetruit < 0 || posRecree < 0) verdict = "CHAINE PERIMEE";
        else if (!reenregistre) verdict = "POIGNEES PERDUES";
        if (verdict[0] == 'S') ++survivants;
        std::cout << nom << ' ' << verdict << '\n';
    }
    std::cout << "SURVIVANTS " << survivants << '\n';
    return 0;
}
