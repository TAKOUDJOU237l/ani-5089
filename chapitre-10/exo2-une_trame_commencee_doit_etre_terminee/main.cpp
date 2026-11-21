// Exercice 2 : une trame commencee doit etre terminee.
#include <cstdio>
#include <string>

enum Etat { REPOS, ATTENDUE, COMMENCEE };

int main() {
    int n = 0;
    if (std::scanf("%d", &n) != 1) return 0;
    Etat etat = REPOS;
    int fautes = 0, bloque = 0;
    for (int i = 1; i <= n; ++i) {
        char buf[64];
        if (std::scanf("%63s", buf) != 1) break;
        const std::string geste = buf;
        std::string verdict = "OK";
        if (geste == "ATTEND") {
            if (etat == REPOS) etat = ATTENDUE;
            else if (etat == ATTENDUE) verdict = "DEJA ATTENDUE";
            else { verdict = "TRAME EN COURS"; if (bloque == 0) bloque = i; }
        } else if (geste == "COMMENCE") {
            if (etat == ATTENDUE) etat = COMMENCEE;
            else if (etat == REPOS) verdict = "PAS ATTENDUE";
            else verdict = "DEJA COMMENCEE";
        } else if (geste == "FINIT") {
            if (etat == COMMENCEE) etat = REPOS;
            else verdict = "PAS COMMENCEE";
        } else {
            continue;   // geste inconnu : hors de l'enonce, ignore
        }
        if (verdict != "OK") ++fautes;   // une faute ne change jamais l'etat
        std::printf("%d %s %s\n", i, geste.c_str(), verdict.c_str());
    }
    std::printf("FAUTES %d\n", fautes);
    std::printf("BLOQUE %d\n", bloque);
    return 0;
}
