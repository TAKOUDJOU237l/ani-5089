// Exercice 4 : ce qu'un vrai casque demande.
// L'echelle est en milliemes. Chaque dimension est tronquee separement,
// puis on compte les pixels des deux yeux.
#include <iostream>
#include <string>

namespace {

const long long kUnite = 1000;      // une echelle de 1000 garde la taille recommandee
const long long kYeux = 2;

long long Echelonner(long long dimension, long long echelle) {
    return dimension * echelle / kUnite;    // division entiere : on tronque vers le bas
}

long long TotalPixels(long long largeur, long long hauteur) {
    return largeur * hauteur * kYeux;
}

} // namespace

int main() {
    long long echelle = kUnite;
    long long budget = 0;
    if (!(std::cin >> echelle >> budget)) {
        return 0;
    }

    int casques = 0;
    std::cin >> casques;

    int tiennent = 0;
    long long economises = 0;

    for (int i = 0; i < casques; ++i) {
        std::string nom;
        long long largeur = 0;
        long long hauteur = 0;
        std::cin >> nom >> largeur >> hauteur;

        const long long largeurRendue = Echelonner(largeur, echelle);
        const long long hauteurRendue = Echelonner(hauteur, echelle);
        const long long total = TotalPixels(largeurRendue, hauteurRendue);
        const bool tient = total <= budget;

        if (tient) {
            ++tiennent;
        }
        // On additionne les economies meme quand le casque depasse.
        economises += TotalPixels(largeur, hauteur) - total;

        std::cout << nom << ' ' << largeurRendue << ' ' << hauteurRendue << ' '
                  << total << ' ' << (tient ? "TIENT" : "DEPASSE") << '\n';
    }

    std::cout << "TIENNENT " << tiennent << '\n';
    std::cout << "ECONOMISES " << economises << '\n';
    return 0;
}
