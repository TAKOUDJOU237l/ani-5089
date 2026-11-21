// Exercice 3 : l'anneau n'avance que quand on dessine.
#include <cstdio>

int main() {
    int images = 0, trames = 0;
    if (std::scanf("%d %d", &images, &trames) != 2) return 0;
    int index = 0, rendues = 0;
    for (int t = 1; t <= trames; ++t) {
        int dessine = 0;
        if (std::scanf("%d", &dessine) != 1) break;
        if (dessine) {
            std::printf("%d OUI 1 %d\n", t, index);
            ++rendues;
            if (images > 0) index = (index + 1) % images;   // l'index ne bouge que sur une trame rendue
        } else {
            std::printf("%d NON 0 -\n", t);
        }
    }
    std::printf("RENDUES %d\n", rendues);
    std::printf("FINS %d\n", trames);   // la fin de trame est appelee a chaque trame
    return 0;
}
