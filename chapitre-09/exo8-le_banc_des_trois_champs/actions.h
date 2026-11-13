#pragma once

// Etat d'une action booleenne : les trois champs du chapitre.
struct EtatAction {
    bool courant = false;   // l'action est vraie a cette image
    bool change = false;    // courant a bascule depuis la synchronisation precedente
    bool actif = false;     // une source materielle est liee
};

// Une action : declaree par son intention (le nom ne sert qu'au journal),
// liee ou non a un organe. On la synchronise une fois par image.
class Action {
public:
    Action(const char *nom, bool lie) : mNom(nom), mLie(lie) {}

    // `presse` : l'etat de la source materielle a cette image.
    void Synchroniser(bool presse) {
        EtatAction suivant;
        suivant.actif = mLie;
        suivant.courant = mLie ? presse : false;   // une action inactive rend courant faux
        suivant.change = suivant.courant != mEtat.courant;   // calcule sur courant, pas sur la touche
        mEtat = suivant;
    }

    const EtatAction &Etat() const { return mEtat; }
    const char *Nom() const { return mNom; }

private:
    const char *mNom;
    bool mLie;
    EtatAction mEtat;
};
