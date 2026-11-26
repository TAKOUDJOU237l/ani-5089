// Exercice 5 : les conditions qui ont fui.
// On compte les conditions de backend zone par zone et on dit ou
// l'abstraction a cede.
#include <iostream>
#include <string>

namespace {

// Quota de conditions admises dans une zone, -1 si la zone est inconnue.
int Quota(const std::string &zone) {
    if (zone == "FABRIQUE") {
        return 1;       // celle qui choisit le backend
    }
    if (zone == "INIT") {
        return 3;       // le champ du descripteur, l'ordre de creation, la provenance des images
    }
    return 0;           // BOUCLE et SCENE : aucune
}

bool ZoneAvecQuota(const std::string &zone) {
    return zone == "FABRIQUE" || zone == "INIT";
}

} // namespace

int main() {
    int fichiers = 0;
    if (!(std::cin >> fichiers)) {
        fichiers = 0;
    }

    int fuites = 0;
    long long branches = 0;

    for (int i = 0; i < fichiers; ++i) {
        std::string nom;
        std::string zone;
        long long conditions = 0;
        std::cin >> nom >> zone >> conditions;
        branches += conditions;

        const char *verdict = "OK";
        if (conditions > 0) {
            if (ZoneAvecQuota(zone)) {
                if (conditions > Quota(zone)) {
                    verdict = "TROP";
                }
            } else {
                verdict = "A FUI";
                ++fuites;
            }
        }
        std::cout << nom << ' ' << verdict << '\n';
    }

    std::cout << "FUITES " << fuites << '\n';
    std::cout << "BRANCHES " << branches << '\n';
    return 0;
}
