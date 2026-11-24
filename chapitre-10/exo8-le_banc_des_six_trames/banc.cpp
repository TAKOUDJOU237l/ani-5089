// Banc des six trames : pas de casque, pas de fenetre. La tete tourne d'un
// degre par trame, et les trames 3 et 4 ne dessinent rien (casque pose).
// Le banc fait tourner DEUX versions du meme scenario et compare leurs sorties :
//   - la version CORRIGEE : la fin de trame est appelee a chaque tour, hors du test de rendu ;
//   - la version FAUTIVE  : la fin de trame est dans le test de rendu.
#include <cstdio>
#include "couches.h"

namespace {

const int kTailleAnneau = 3;                                       // images de la chaine
const bool kDessine[6] = { true, true, false, false, true, true }; // trames 3 et 4 : casque pose

struct Ligne {
    int rend = 0;        // on a dessine cette trame
    int couches = 0;     // couches soumises
    int index = -1;      // index d'image emprunte, -1 : aucune
    int fins = 0;        // fins de trame soumises, cumulees
    bool refusee = false; // l'attente a ete refusee : une trame etait restee ouverte
};

// Fait tourner les six trames. `finDansLeTest` choisit la version.
void Executer(bool finDansLeTest, Ligne lignes[6]) {
    int index = 0;           // la prochaine image a emprunter
    int fins = 0;
    bool ouverte = false;    // une trame commencee et pas terminee
    double lacet = 0.0;      // angle de la tete, en degres

    for (int t = 1; t <= 6; ++t) {
        lacet += 1.0;                                 // la tete tourne d'un degre par trame
        Ligne &l = lignes[t - 1];

        // Attente de la trame : refusee tant qu'une trame est restee ouverte.
        // (Sur un vrai casque, elle se bloquerait.)
        if (ouverte) {
            l.refusee = true;
            l.fins = fins;
            continue;
        }
        ouverte = true;                               // debut de trame

        if (kDessine[t - 1]) {
            const int utilise = index;                // on emprunte, on rend, on libere
            index = (index + 1) % kTailleAnneau;      // l'index n'avance QUE sur une trame rendue

            CoucheProjection couche;                  // sur la pile, consommee a l'appel
            couche.espace = Espace::STAGE;
            for (int oeil = 0; oeil < 2; ++oeil) {
                couche.vues[oeil].lacetDegres = lacet;    // la pose du RENDU
                couche.vues[oeil].chaine = oeil;
                couche.vues[oeil].indexImage = utilise;
            }
            l.rend = 1; l.couches = 1; l.index = utilise;

            if (finDansLeTest) { ++fins; ouverte = false; }   // VERSION FAUTIVE
        }

        // VERSION CORRIGEE : la fin de trame est HORS du test de rendu, appelee a
        // chaque tour, y compris aux trames 3 et 4, avec zero couche.
        if (!finDansLeTest) { ++fins; ouverte = false; }
        l.fins = fins;
    }
}

void Imprimer(const char *titre, const Ligne lignes[6]) {
    std::printf("%s\n", titre);
    std::printf("image | on rend | couches soumises | index d'image | fins de trame\n");
    for (int t = 1; t <= 6; ++t) {
        const Ligne &l = lignes[t - 1];
        if (l.index >= 0)
            std::printf("%5d | %7d | %16d | %13d | %13d\n", t, l.rend, l.couches, l.index, l.fins);
        else
            std::printf("%5d | %7d | %16d | %13s | %13d\n", t, l.rend, l.couches, "-", l.fins);
    }
}

} // namespace

int main() {
    Ligne corrigee[6], fautive[6];
    Executer(false, corrigee);
    Executer(true, fautive);

    Imprimer("version corrigee (la fin de trame est hors du test de rendu)", corrigee);
    std::printf("\n");
    Imprimer("version fautive (la fin de trame est dans le test de rendu)", fautive);

    std::printf("\ncomparaison, trame par trame\n");
    std::printf("image | corrigee: on rend, index, fins de trame | fautive: on rend, index, fins de trame, attente\n");
    for (int t = 1; t <= 6; ++t) {
        const Ligne &c = corrigee[t - 1];
        const Ligne &f = fautive[t - 1];
        char ic[8], iff[8];
        std::snprintf(ic, sizeof ic, c.index >= 0 ? "%d" : "-", c.index);
        std::snprintf(iff, sizeof iff, f.index >= 0 ? "%d" : "-", f.index);
        std::printf("%5d | %17d %5s %13d | %16d %5s %13d %s\n", t,
                    c.rend, ic, c.fins, f.rend, iff, f.fins, f.refusee ? "REFUSEE" : "ok");
    }
    return 0;
}