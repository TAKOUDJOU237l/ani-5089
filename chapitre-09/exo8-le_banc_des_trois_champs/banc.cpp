// Banc des trois champs : pas de casque, pas de manette, pas de fenetre.
#include <cstdio>
#include "actions.h"

int main() {
    Action saisir("saisir", true);   // liee a la touche G
    Action tirer("tirer", true);     // liee a la touche T
    Action morte("morte", false);    // liee a rien

    // Etat des touches, image 1 a 6. G est tenue de l'image 2 a 4, T est
    // pressee aux images 3 et 6, la touche de "morte" est pressee image 3.
    const bool touchG[6] = { false, true, true, true, false, false };
    const bool touchT[6] = { false, false, true, false, false, true };
    const bool touchM[6] = { false, false, true, false, false, false };

    std::printf("image | saisir: courant change actif | tirer: courant change actif | morte: courant change actif\n");
    for (int i = 0; i < 6; ++i) {
        saisir.Synchroniser(touchG[i]);
        tirer.Synchroniser(touchT[i]);
        morte.Synchroniser(touchM[i]);
        const EtatAction &s = saisir.Etat();
        const EtatAction &t = tirer.Etat();
        const EtatAction &m = morte.Etat();
        std::printf("%5d | %14d %6d %5d | %14d %6d %5d | %14d %6d %5d\n", i + 1,
                    s.courant, s.change, s.actif, t.courant, t.change, t.actif,
                    m.courant, m.change, m.actif);
    }
    return 0;
}