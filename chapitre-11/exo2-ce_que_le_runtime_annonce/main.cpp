// Exercice 2 : ce que le runtime annonce.
// Une fonctionnalite est DISPONIBLE si l'extension qu'elle exige est offerte,
// sinon elle est RENONCEE : l'application continue sans elle.
#include <iostream>
#include <set>
#include <string>
#include <vector>

namespace {

struct Fonctionnalite {
    std::string nom;
    std::string extension;
};

} // namespace

int main() {
    int offertes = 0;
    if (!(std::cin >> offertes)) {
        offertes = 0;
    }

    // Comparaison exacte, majuscules comprises : on garde les noms tels quels.
    std::set<std::string> annoncees;
    for (int i = 0; i < offertes; ++i) {
        std::string extension;
        std::cin >> extension;
        annoncees.insert(extension);
    }

    int voulues = 0;
    if (!(std::cin >> voulues)) {
        voulues = 0;
    }

    std::vector<Fonctionnalite> fonctions;
    for (int i = 0; i < voulues; ++i) {
        Fonctionnalite f;
        std::cin >> f.nom >> f.extension;
        fonctions.push_back(f);
    }

    int renoncees = 0;
    for (const Fonctionnalite &f : fonctions) {
        const bool disponible = annoncees.count(f.extension) > 0;
        if (!disponible) {
            ++renoncees;
        }
        std::cout << f.nom << ' ' << (disponible ? "DISPONIBLE" : "RENONCEE") << '\n';
    }

    // OFFERTES compte ce que le runtime annonce, pas ce que l'application emploie.
    std::cout << "OFFERTES " << offertes << '\n';
    std::cout << "RENONCEES " << renoncees << '\n';
    return 0;
}
