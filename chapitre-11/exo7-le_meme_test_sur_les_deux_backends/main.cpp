// Exercice 7 : le meme test, sur les deux backends.
// On compare les valeurs d'un test numerique lance sur le simulateur et sur
// le casque : l'ecart absolu decide IDENTIQUE, TOLERE ou DIVERGE.
#include <iostream>
#include <string>

namespace {

long long ValeurAbsolue(long long v) {
    return v < 0 ? -v : v;
}

} // namespace

int main() {
    long long tolerance = 0;
    int mesures = 0;
    if (!(std::cin >> tolerance)) {
        tolerance = 0;
    }
    if (!(std::cin >> mesures)) {
        mesures = 0;
    }

    int divergents = 0;

    for (int i = 0; i < mesures; ++i) {
        std::string nom;
        long long valeurSim = 0;
        long long valeurCasque = 0;
        std::cin >> nom >> valeurSim >> valeurCasque;

        const long long ecart = ValeurAbsolue(valeurSim - valeurCasque);
        const char *verdict = "DIVERGE";
        if (ecart == 0) {
            verdict = "IDENTIQUE";          // l'ecart nul passe avant la tolerance
        } else if (ecart <= tolerance) {
            verdict = "TOLERE";
        } else {
            ++divergents;
        }
        std::cout << nom << ' ' << ecart << ' ' << verdict << '\n';
    }

    // Aucune mesure du tout ne prouve rien.
    const int prouve = (mesures > 0 && divergents == 0) ? 1 : 0;
    std::cout << "DIVERGENTS " << divergents << '\n';
    std::cout << "PROUVE " << prouve << '\n';
    return 0;
}
