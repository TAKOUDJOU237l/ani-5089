// =============================================================================
//  Exercice 10 (chapitre 11) : le test numerique qui prouve.
//                                                      *** VERSION SIMULEE ***
//
//  Un test SANS TETE : il cree une session sans fenetre (aucun NkWindow, aucun
//  peripherique graphique), pose une pose de tete scriptee, avance de dix
//  trames, et releve cinq entiers a chaque trame : le numero de trame, l'instant
//  predit (microsecondes), l'index d'image, x et y de l'oeil gauche (millimetres).
//
//  Il n'y a pas de casque et pas de module XR dans le kit : la session est une
//  SIMULATION ecrite ici. Elle n'est pas une coquille : elle possede
//    - une horloge, qui predit l'instant d'affichage de chaque trame (72 Hz,
//      comme l'exemple de l'enonce : trame 3 -> 41666 us) ;
//    - une chaine d'images, ou l'on emprunte puis rend chaque image ;
//    - le protocole du chapitre 10 : attente, debut, fin, et elle REFUSE un
//      appel hors de l'ordre ;
//    - un fournisseur de pose, echantillonne a l'instant PREDIT, jamais a
//      partir du numero de trame.
//  Ce qu'elle ne fait pas : aucun rendu, aucune vraie prediction de runtime.
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
#include <vector>

namespace {

const int kTrames = 10;                        // le test avance de dix trames
const long long kHertz = 72;                   // cadence de l'exemple : 41666 us a la trame 3
const long long kMicrosParSeconde = 1000000;
const int kDemiEcartYeuxMm = 31;               // 62 mm entre les yeux : l'oeil gauche est a -31
const int kHauteurYeuxMm = 1700;
const long long kVitesseTeteMmParSeconde = 200;   // la tete avance de 20 cm par seconde

// Une configuration : ce que l'on fait varier entre les deux executions.
struct Configuration {
    const char *nom;
    int imagesChaine;                          // taille de la chaine d'images
};

const Configuration kConfigA = {"A : session sans fenetre, chaine de 3 images", 3};
const Configuration kConfigB = {"B : session sans fenetre, chaine de 2 images", 2};

// Le fournisseur de pose : une pose SCRIPTEE, fonction du TEMPS et non de la trame.
// Il rend la position de l'oeil gauche, en millimetres, a un instant en microsecondes.
void PoseScriptee(long long instantMicros, long long &xMm, long long &yMm) {
    const long long teteX = instantMicros * kVitesseTeteMmParSeconde / kMicrosParSeconde;
    xMm = teteX - kDemiEcartYeuxMm;
    yMm = kHauteurYeuxMm;
}

// Ce que l'attente de trame rend : la trame et l'instant predit.
struct EtatTrame {
    int trame = 0;
    long long instantPredit = 0;
};

// Une session sans fenetre.
class SessionSansFenetre {
public:
    explicit SessionSansFenetre(const Configuration &c)
        : config_(c), imageEmpruntee_(c.imagesChaine, false) {}

    // Attente : refusee si une trame est restee ouverte.
    bool Attendre(EtatTrame &etat) {
        if (ouverte_) {
            ++fautes_;
            return false;
        }
        ++trame_;
        etat.trame = trame_;
        // L'instant d'affichage predit : la fin de la trame courante.
        etat.instantPredit = (long long)trame_ * kMicrosParSeconde / kHertz;
        return true;
    }

    bool Commencer() {
        if (ouverte_) {
            ++fautes_;
            return false;
        }
        ouverte_ = true;
        return true;
    }

    // La pose de l'oeil gauche, echantillonnee a l'instant predit.
    bool LocaliserOeil(long long instantPredit, long long &xMm, long long &yMm) {
        if (!ouverte_) {
            ++fautes_;
            return false;
        }
        PoseScriptee(instantPredit, xMm, yMm);
        return true;
    }

    // On emprunte la prochaine image de la chaine : refuse si elle est deja empruntee.
    bool EmprunterImage(int &index) {
        if (!ouverte_ || imageEmpruntee_[prochaine_]) {
            ++fautes_;
            return false;
        }
        index = prochaine_;
        imageEmpruntee_[prochaine_] = true;
        prochaine_ = (prochaine_ + 1) % config_.imagesChaine;   // n'avance que sur une trame rendue
        return true;
    }

    bool RendreImage(int index) {
        if (!imageEmpruntee_[index]) {
            ++fautes_;
            return false;
        }
        imageEmpruntee_[index] = false;
        return true;
    }

    bool Finir() {
        if (!ouverte_) {
            ++fautes_;
            return false;
        }
        ouverte_ = false;
        return true;
    }

    int Fautes() const { return fautes_; }

private:
    Configuration config_;
    std::vector<bool> imageEmpruntee_;
    int prochaine_ = 0;
    int trame_ = 0;
    bool ouverte_ = false;
    int fautes_ = 0;
};

// Execute le test : dix trames, chacune par le protocole complet.
int Releve(const Configuration &config, const char *fichier) {
    std::FILE *sortie = std::fopen(fichier, "w");
    if (sortie == nullptr) {
        std::fprintf(stderr, "impossible d'ecrire %s\n", fichier);
        return 1;
    }
    SessionSansFenetre session(config);
    for (int i = 0; i < kTrames; ++i) {
        EtatTrame etat;
        long long x = 0;
        long long y = 0;
        int image = -1;
        const bool ok = session.Attendre(etat) && session.Commencer()
                        && session.LocaliserOeil(etat.instantPredit, x, y)
                        && session.EmprunterImage(image)
                        && session.RendreImage(image) && session.Finir();
        if (!ok) {
            std::fprintf(stderr, "la session a refuse un appel a la trame %d\n", i + 1);
            std::fclose(sortie);
            return 1;
        }
        std::fprintf(sortie, "trame : %d, instant : %lld, image : %d, x : %lld, y : %lld\n",
                     etat.trame, etat.instantPredit, image, x, y);
        std::printf("trame : %d, instant : %lld, image : %d, x : %lld, y : %lld\n",
                    etat.trame, etat.instantPredit, image, x, y);
    }
    std::fclose(sortie);
    std::printf("configuration %s : %d fautes de protocole -> %s\n", config.nom, session.Fautes(), fichier);
    return session.Fautes() == 0 ? 0 : 1;
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