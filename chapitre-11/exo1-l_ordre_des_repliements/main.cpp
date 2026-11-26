// Exercice 1 : l'ordre des repliements.
// On essaie quatre chemins dans un ordre fixe : ENV, LOADER, DIRECT, SIM.
// Le premier qui aboutit est retenu, les suivants ne sont pas essayes.
#include <iostream>
#include <string>

namespace {

const int kNombreChemins = 4;
const char *const kNomsChemins[kNombreChemins] = {"ENV", "LOADER", "DIRECT", "SIM"};
const char *const kAucun = "AUCUN";

// Rend l'indice du premier chemin qui aboutit, ou -1 si aucun n'aboutit.
int PremierQuiAboutit(const int aboutit[kNombreChemins]) {
    for (int i = 0; i < kNombreChemins; ++i) {
        if (aboutit[i] != 0) {
            return i;
        }
    }
    return -1;
}

} // namespace

int main() {
    int machines = 0;
    if (!(std::cin >> machines)) {
        machines = 0;
    }

    int casques = 0;
    int replis = 0;
    int refus = 0;

    for (int m = 0; m < machines; ++m) {
        std::string nom;
        int aboutit[kNombreChemins] = {0, 0, 0, 0};
        std::cin >> nom;
        for (int i = 0; i < kNombreChemins; ++i) {
            std::cin >> aboutit[i];
        }

        const int retenu = PremierQuiAboutit(aboutit);
        if (retenu < 0) {
            ++refus;
            std::cout << nom << ' ' << kAucun << '\n';
        } else if (retenu == kNombreChemins - 1) {
            ++replis;       // le simulateur : un repli, pas un casque
            std::cout << nom << ' ' << kNomsChemins[retenu] << '\n';
        } else {
            ++casques;      // ENV, LOADER ou DIRECT : un vrai casque
            std::cout << nom << ' ' << kNomsChemins[retenu] << '\n';
        }
    }

    std::cout << "CASQUES " << casques << '\n';
    std::cout << "REPLIS " << replis << '\n';
    std::cout << "REFUS " << refus << '\n';
    return 0;
}
