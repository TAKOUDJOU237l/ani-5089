// Exercice 5 : la couche que le simulateur refuse en silence.
#include <cstdio>
#include <string>

int main() {
    int n = 0;
    if (std::scanf("%d", &n) != 1) return 0;
    int refusees = 0, averties = 0;
    for (int i = 0; i < n; ++i) {
        char nom[128];
        int commence = 0, rend = 0, gauche = 0, droite = 0;
        if (std::scanf("%127s %d %d %d %d", nom, &commence, &rend, &gauche, &droite) != 5) break;
        std::string verdict;
        if (!commence) {                        // le debut de trame se teste en premier
            verdict = "REFUSEE";
            ++refusees;
        } else if (!rend) {
            verdict = (gauche || droite) ? "COUCHE SANS RENDU" : "OK";
        } else if (gauche && droite) {
            verdict = "OK";
        } else if (!gauche && !droite) {
            verdict = "SOUMISSION VIDE";
        } else {
            verdict = gauche ? "OEIL NOIR D" : "OEIL NOIR G";
        }
        if (verdict != "OK" && verdict != "REFUSEE") ++averties;   // REFUSEE ne compte pas ici
        std::printf("%s %s\n", nom, verdict.c_str());
    }
    std::printf("REFUSEES %d\n", refusees);
    std::printf("AVERTIES %d\n", averties);
    return 0;
}
