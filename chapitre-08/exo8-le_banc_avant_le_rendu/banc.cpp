// Banc de la chaine d'echange : pas de fenetre, pas de carte, pas de casque.
#include <cstdio>
#include "chaine.h"

// Pour comparaison seulement : ce que serait une chaine PERMISSIVE, qui ne
// refuse jamais un emprunt. Elle compte les emprunts faits sur une image
// qui est deja sortie.
struct ChainePermissive {
    uint32_t suivant = 0u;
    bool sortie = false;
    int surImageDejaSortie = 0;
    bool AcquireImage(uint32_t &index) {
        if (sortie) ++surImageDejaSortie;
        sortie = true;
        index = suivant;
        return true;
    }
};

int main() {
    ChaineEchange chaine;
    if (!chaine.Creer(3u)) {
        std::printf("creation refusee\n");
        return 1;
    }

    // Six tours sur un anneau de trois : l'index doit boucler 0 1 2 0 1 2.
    for (int tour = 1; tour <= 6; ++tour) {
        uint32_t index = 99u;
        const bool pris = chaine.AcquireImage(index);
        const bool rendu = chaine.ReleaseImage();
        std::printf("tour %d : emprunte %u (%s), rendu (%s), a soumettre %u\n",
                    tour, index, pris ? "ok" : "refuse", rendu ? "ok" : "refuse",
                    chaine.GetLastReleasedIndex());
    }

    // Les trois refus.
    uint32_t index = 0u;
    chaine.AcquireImage(index);
    const bool doubleEmprunt = chaine.AcquireImage(index);
    std::printf("refus 1, emprunter deux fois : %s\n", doubleEmprunt ? "ACCEPTE (faute)" : "refuse");
    chaine.ReleaseImage();
    const bool renduSansEmprunt = chaine.ReleaseImage();
    std::printf("refus 2, rendre sans avoir emprunte : %s\n", renduSansEmprunt ? "ACCEPTE (faute)" : "refuse");
    ChaineEchange trop;
    const bool tropGrande = trop.Creer(ChaineEchange::kMaxImages + 1u);
    std::printf("refus 3, chaine de %u images (maximum %u) : %s\n",
                ChaineEchange::kMaxImages + 1u, ChaineEchange::kMaxImages,
                tropGrande ? "ACCEPTEE (faute)" : "refusee");

    // Le meme programme fautif (six emprunts d'affilee, sans jamais rendre),
    // devant la chaine stricte puis devant la chaine permissive.
    ChaineEchange stricte;
    stricte.Creer(3u);
    ChainePermissive permissive;
    int accepteStricte = 0, refuseStricte = 0, acceptePermissive = 0;
    for (int tour = 1; tour <= 6; ++tour) {
        uint32_t i = 0u;
        if (stricte.AcquireImage(i)) ++accepteStricte; else ++refuseStricte;
        if (permissive.AcquireImage(i)) ++acceptePermissive;
    }
    std::printf("comparaison, six emprunts d'affilee sans rendre : chaine stricte %d acceptes %d refuses ; "
                "chaine permissive %d acceptes dont %d sur une image deja sortie\n",
                accepteStricte, refuseStricte, acceptePermissive, permissive.surImageDejaSortie);

    return (doubleEmprunt || renduSansEmprunt || tropGrande) ? 1 : 0;
}