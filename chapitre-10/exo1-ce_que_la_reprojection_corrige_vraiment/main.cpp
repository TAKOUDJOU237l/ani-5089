// Exercice 1 : ce que la reprojection corrige vraiment.
#include <cstdio>
#include <cstdlib>

int main() {
    int n = 0;
    if (std::scanf("%d", &n) != 1) return 0;
    int justes = 0, pires = 0;
    for (int i = 0; i < n; ++i) {
        long rendu = 0, reel = 0, soumis = 0;
        if (std::scanf("%ld %ld %ld", &rendu, &reel, &soumis) != 3) break;
        const long sans = rendu - reel;     // l'erreur sans reprojection
        const long avec = rendu - soumis;   // l'erreur qui reste a l'ecran
        const char *verdict;
        if (avec == 0) {                    // le cas nul d'abord
            verdict = "JUSTE";
            ++justes;
        } else if (std::labs(avec) < std::labs(sans)) {
            verdict = "PARTIEL";
        } else if (std::labs(avec) == std::labs(sans)) {
            verdict = "SANS EFFET";
        } else {
            verdict = "PIRE";
            ++pires;
        }
        std::printf("SANS %ld AVEC %ld %s\n", sans, avec, verdict);
    }
    std::printf("JUSTES %d\n", justes);
    std::printf("PIRES %d\n", pires);
    return 0;
}
