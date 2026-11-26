// Exercice 3 : session d'abord, device ensuite.
// On suit quatre drapeaux et on dit ou la liaison echouera.
#include <iostream>
#include <string>

namespace {

struct Etat {
    bool session = false;      // la session existe
    bool exigences = false;    // les exigences ont ete demandees
    bool device = false;       // le peripherique existe
    bool complet = false;      // le peripherique est complet
};

// Applique un geste a l'etat et rend le verdict : "OK" ou la faute.
std::string Appliquer(const std::string &geste, Etat &e) {
    if (geste == "SESSION") {
        if (e.session) {
            return "DEJA CREEE";
        }
        e.session = true;
        return "OK";
    }
    if (geste == "EXIGENCES") {
        if (!e.session) {
            return "PAS DE SESSION";
        }
        e.exigences = true;
        return "OK";
    }
    if (geste == "DEVICE") {
        if (e.device) {
            return "DEJA CREE";
        }
        if (!e.session) {
            return "SANS SESSION";          // aucun peripherique n'est cree
        }
        e.device = true;
        if (!e.exigences) {
            e.complet = false;              // cree, mais incomplet : c'est tout le probleme
            return "EXIGENCES IGNOREES";
        }
        e.complet = true;
        return "OK";
    }
    if (geste == "LIER") {
        if (!e.device) {
            return "PAS DE DEVICE";
        }
        if (!e.complet) {
            return "LIAISON ECHOUE";
        }
        return "OK";
    }
    return "OK";
}

} // namespace

int main() {
    int gestes = 0;
    if (!(std::cin >> gestes)) {
        gestes = 0;
    }

    Etat etat;
    int fautes = 0;
    int repli = 0;      // reste a 1 une fois leve

    for (int i = 1; i <= gestes; ++i) {
        std::string geste;
        std::cin >> geste;
        const std::string verdict = Appliquer(geste, etat);
        if (verdict != "OK") {
            ++fautes;
        }
        if (verdict == "LIAISON ECHOUE") {
            repli = 1;
        }
        std::cout << i << ' ' << geste << ' ' << verdict << '\n';
    }

    std::cout << "FAUTES " << fautes << '\n';
    std::cout << "REPLI " << repli << '\n';
    return 0;
}
