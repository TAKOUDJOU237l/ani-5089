// Exercice 7 : ce que la couche quad fait gagner au texte.
#include <cstdio>

int main() {
    long rendu = 0, deformation = 0, reprojection = 0, seuil = 0;
    int n = 0;
    if (std::scanf("%ld %ld %ld %ld %d", &rendu, &deformation, &reprojection, &seuil, &n) != 5) return 0;
    int gagnes = 0, perdus = 0;
    for (int i = 0; i < n; ++i) {
        char nom[128];
        long pixels = 0;
        if (std::scanf("%127s %ld", nom, &pixels) != 2) break;
        long scene = pixels * rendu / 1000;          // on tronque a CHAQUE etape
        scene = scene * deformation / 1000;
        scene = scene * reprojection / 1000;
        const long quad = pixels;                    // la couche quad garde ses pixels
        const bool lisibleScene = scene >= seuil;    // comparaison large : au seuil, c'est lisible
        const bool lisibleQuad = quad >= seuil;
        std::printf("%s %ld %ld %s %s\n", nom, scene, quad,
                    lisibleScene ? "LISIBLE" : "FLOU", lisibleQuad ? "LISIBLE" : "FLOU");
        if (!lisibleScene && lisibleQuad) ++gagnes;
        if (lisibleScene && !lisibleQuad) ++perdus;
    }
    std::printf("GAGNES %d\n", gagnes);
    std::printf("PERDUS %d\n", perdus);
    return 0;
}
