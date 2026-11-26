// Exercice 6 : trois niveaux, et le nettoyage entre deux.
// On descend la pile CASQUE, CARTE, LOGICIEL et on repere les etats hybrides.
#include <iostream>
#include <string>

namespace {

const int kNiveaux = 3;
const char *const kNomsNiveaux[kNiveaux] = {"CASQUE", "CARTE", "LOGICIEL"};

} // namespace

int main() {
    int machines = 0;
    if (!(std::cin >> machines)) {
        machines = 0;
    }

    int casques = 0;
    int hybrides = 0;

    for (int m = 0; m < machines; ++m) {
        std::string nom;
        int aboutit[kNiveaux] = {0, 0, 0};
        int nettoie = 0;
        std::cin >> nom;
        for (int i = 0; i < kNiveaux; ++i) {
            std::cin >> aboutit[i];
        }
        std::cin >> nettoie;

        // Le nombre de descentes est le nombre de niveaux essayes en vain.
        int descentes = 0;
        while (descentes < kNiveaux && aboutit[descentes] == 0) {
            ++descentes;
        }
        const bool aucun = descentes == kNiveaux;      // rien n'aboutit : trois descentes
        const std::string niveau = aucun ? "AUCUN" : kNomsNiveaux[descentes];

        // Hybride : on a descendu au moins une fois et rien n'a ete nettoye.
        const bool hybride = descentes >= 1 && nettoie == 0;

        if (!aucun && descentes == 0) {
            ++casques;
        }
        if (hybride) {
            ++hybrides;
        }
        std::cout << nom << ' ' << niveau << ' ' << descentes << ' '
                  << (hybride ? "HYBRIDE" : "PROPRE") << '\n';
    }

    std::cout << "CASQUES " << casques << '\n';
    std::cout << "HYBRIDES " << hybrides << '\n';
    return 0;
}
