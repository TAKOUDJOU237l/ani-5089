// =============================================================================
//  Exercice 10 (chapitre 11) : le test numerique qui prouve.
//                                                      *** VERSION SIMULEE ***
//
//  Un test SANS TETE : il cree une session sans fenetre (aucun NkWindow, aucun
//  peripherique graphique), pose une pose de tete scriptee, avance de dix
//  trames, et releve quatre entiers a chaque trame :
//      instant predit (en microsecondes), index d'image, x et y de l'oeil gauche
//      (en millimetres).
//  Il n'y a pas de casque et pas de module XR dans le kit : la session est une
//  simulation ecrite ici, qui suit le protocole du chapitre 10 (attente, debut,
//  fin de chaque trame).
//
//  Trois modes, trois executions distinctes :
//      test sim    valeurs-sim.txt      configuration A : chaine de 3 images
//      test autre  valeurs-autre.txt    configuration B : chaine de 2 images
//      test comparer valeurs-sim.txt valeurs-autre.txt comparaison.txt
//  Compilation : clang++ -std=c++17 -Wall test.cpp -o test
// =============================================================================
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

namespace {

const int kTrames = 10;                  // le test avance de dix trames
const long long kHertz = 60;             // la cadence est la meme sur les deux configurations
const long long kMicrosParSeconde = 1000000;
const int kDemiEcartYeuxMm = 31;         // 62 mm entre les yeux : l'oeil gauche est a -31
const int kHauteurYeuxMm = 1700;
const int kPasMarcheMm = 5;              // la tete avance de 5 mm par trame

// Une configuration : ce que l'on fait varier entre les deux executions.
struct Configuration {
    const char *nom;
    int imagesChaine;                    // taille de la chaine d'images
};

const Configuration kConfigA = {"A : session sans fenetre, chaine de 3 images", 3};
const Configuration kConfigB = {"B : session sans fenetre, chaine de 2 images", 2};

// Une session sans fenetre : elle compte les trames, comme le simulateur du chapitre.
class SessionSansFenetre {
public:
    explicit SessionSansFenetre(const Configuration &c) : config_(c) {}

    // La pose de tete scriptee : une fonction de la trame, jamais de l'horloge.
    void PoseDeLOeilGauche(int trame, int &xMm, int &yMm) const {
        const int teteX = kPasMarcheMm * (trame - 1);
        xMm = teteX - kDemiEcartYeuxMm;
        yMm = kHauteurYeuxMm;
    }

    // L'instant d'affichage predit pour cette trame : une periode par trame.
    long long InstantPredit(int trame) const {
        return (long long)trame * kMicrosParSeconde / kHertz;
    }

    // Toutes les trames sont dessinees, donc l'index avance a chaque trame.
    int IndexDImage(int trame) const {
        return (trame - 1) % config_.imagesChaine;
    }

    // Un tour : attente, debut, fin. Une trame commencee est toujours terminee.
    void Avancer() {
        ++trames_;
        ouverte_ = true;
        ouverte_ = false;
    }

    int Trames() const { return trames_; }
    const Configuration &Config() const { return config_; }

private:
    Configuration config_;
    int trames_ = 0;
    bool ouverte_ = false;
};

// Execute le test et ecrit dix lignes dans le fichier donne (et a l'ecran).
int Releve(const Configuration &config, const char *fichier) {
    std::FILE *sortie = std::fopen(fichier, "w");
    if (sortie == nullptr) {
        std::fprintf(stderr, "impossible d'ecrire %s\n", fichier);
        return 1;
    }
    SessionSansFenetre session(config);
    for (int i = 0; i < kTrames; ++i) {
        session.Avancer();
        const int trame = session.Trames();
        int x = 0;
        int y = 0;
        session.PoseDeLOeilGauche(trame, x, y);
        const long long instant = session.InstantPredit(trame);
        const int image = session.IndexDImage(trame);
        std::fprintf(sortie, "trame : %d, instant : %lld, image : %d, x : %d, y : %d\n",
                     trame, instant, image, x, y);
        std::printf("trame : %d, instant : %lld, image : %d, x : %d, y : %d\n",
                    trame, instant, image, x, y);
    }
    std::fclose(sortie);
    std::printf("configuration %s -> %s\n", config.nom, fichier);
    return 0;
}

struct Ligne {
    long long trame = 0;
    long long instant = 0;
    long long image = 0;
    long long x = 0;
    long long y = 0;
};

bool LireReleve(const char *fichier, std::vector<Ligne> &lignes) {
    std::FILE *f = std::fopen(fichier, "r");
    if (f == nullptr) {
        return false;
    }
    Ligne l;
    while (std::fscanf(f, " trame : %lld, instant : %lld, image : %lld, x : %lld, y : %lld",
                       &l.trame, &l.instant, &l.image, &l.x, &l.y) == 5) {
        lignes.push_back(l);
    }
    std::fclose(f);
    return true;
}

long long Absolu(long long v) {
    return v < 0 ? -v : v;
}

// Compare deux releves deja ecrits : un ecart par valeur et par trame.
int Comparer(const char *fichierA, const char *fichierB, const char *fichierSortie) {
    std::vector<Ligne> a;
    std::vector<Ligne> b;
    if (!LireReleve(fichierA, a) || !LireReleve(fichierB, b)) {
        std::fprintf(stderr, "impossible de lire les releves\n");
        return 1;
    }
    if (a.size() != b.size()) {
        std::fprintf(stderr, "les deux releves n'ont pas le meme nombre de trames\n");
        return 1;
    }
    std::FILE *sortie = std::fopen(fichierSortie, "w");
    if (sortie == nullptr) {
        std::fprintf(stderr, "impossible d'ecrire %s\n", fichierSortie);
        return 1;
    }
    for (size_t i = 0; i < a.size(); ++i) {
        const long long ei = Absolu(a[i].instant - b[i].instant);
        const long long eg = Absolu(a[i].image - b[i].image);
        const long long ex = Absolu(a[i].x - b[i].x);
        const long long ey = Absolu(a[i].y - b[i].y);
        std::fprintf(sortie, "trame : %lld, ecart instant : %lld, ecart image : %lld, ecart x : %lld, ecart y : %lld\n",
                     a[i].trame, ei, eg, ex, ey);
        std::printf("trame : %lld, ecart instant : %lld, ecart image : %lld, ecart x : %lld, ecart y : %lld\n",
                    a[i].trame, ei, eg, ex, ey);
    }
    std::fclose(sortie);
    return 0;
}

} // namespace

int main(int argc, char **argv) {
    if (argc == 3 && std::strcmp(argv[1], "sim") == 0) {
        return Releve(kConfigA, argv[2]);
    }
    if (argc == 3 && std::strcmp(argv[1], "autre") == 0) {
        return Releve(kConfigB, argv[2]);
    }
    if (argc == 5 && std::strcmp(argv[1], "comparer") == 0) {
        return Comparer(argv[2], argv[3], argv[4]);
    }
    std::fprintf(stderr, "usage : test sim <fichier> | test autre <fichier> | test comparer <a> <b> <sortie>\n");
    return 2;
}
