// Exercice 6 : enterre, ou en levitation.
#include <cstdio>
#include <string>

int main() {
    long hauteur = 0;
    int n = 0;
    if (std::scanf("%ld %d", &hauteur, &n) != 2) return 0;
    int decalees = 0;
    for (int i = 0; i < n; ++i) {
        char nom[128], pose[32], couche[32];
        if (std::scanf("%127s %31s %31s", nom, pose, couche) != 3) break;
        const std::string p = pose, c = couche;
        long decalage = 0;
        const char *verdict = "OK";
        if (p != c) {
            if (p == "STAGE") { decalage = hauteur; verdict = "LEVITATION"; }   // STAGE declare en LOCAL : la scene monte
            else              { decalage = -hauteur; verdict = "ENTERRE"; }    // LOCAL declare en STAGE : elle descend
            ++decalees;                     // le verdict decide, meme si la hauteur est nulle
        }
        std::printf("%s %ld %s\n", nom, decalage, verdict);
    }
    std::printf("DECALEES %d\n", decalees);
    return 0;
}
