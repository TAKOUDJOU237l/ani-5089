// Banc des six trames : pas de casque, pas de fenetre. La tete tourne d'un
// degre par trame, et les trames 3 et 4 ne dessinent rien (casque pose).
#include <cstdio>
#include "couches.h"

int main() {
    const int tailleAnneau = 3;                    // images de la chaine
    const bool dessine[6] = { true, true, false, false, true, true };

    int index = 0;                                 // la prochaine image a emprunter
    int fins = 0;                                  // fins de trame soumises
    double lacet = 0.0;                            // angle de la tete, en degres

    std::printf("image | on rend | couches soumises | index d'image | fins de trame\n");
    for (int t = 1; t <= 6; ++t) {
        lacet += 1.0;                              // la tete tourne d'un degre par trame
        int couches = 0;
        int utilise = -1;

        if (dessine[t - 1]) {
            utilise = index;                       // on emprunte, on rend, on libere
            index = (index + 1) % tailleAnneau;    // l'index n'avance QUE sur une trame rendue

            CoucheProjection couche;               // sur la pile, consommee a l'appel
            couche.espace = Espace::STAGE;
            for (int oeil = 0; oeil < 2; ++oeil) {
                couche.vues[oeil].lacetDegres = lacet;   // la pose du RENDU
                couche.vues[oeil].chaine = oeil;
                couche.vues[oeil].indexImage = utilise;
            }
            couches = 1;
        }

        // La fin de trame est HORS du test de rendu : appelee a chaque tour,
        // y compris aux trames 3 et 4, avec zero couche.
        ++fins;

        if (utilise >= 0)
            std::printf("%5d | %7d | %16d | %13d | %13d\n", t, 1, couches, utilise, fins);
        else
            std::printf("%5d | %7d | %16d | %13s | %13d\n", t, 0, couches, "-", fins);
    }
    return 0;
}
