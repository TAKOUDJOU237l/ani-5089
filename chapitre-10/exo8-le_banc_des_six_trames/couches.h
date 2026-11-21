#pragma once

// Les deux espaces de reference d'une couche : l'origine est au plancher (STAGE)
// ou a la hauteur des yeux (LOCAL).
enum class Espace { LOCAL, STAGE };

// Une vue, celle d'un oeil : l'angle de la tete (lacet, en degres) avec lequel
// l'image de cet oeil a ete DESSINEE, et la chaine d'images qui la porte.
struct Vue {
    double lacetDegres = 0.0;   // la pose du rendu, pas la pose fraiche
    int    chaine = -1;         // -1 : aucune chaine pour cet oeil
    int    indexImage = -1;     // l'image relachee de cette chaine
};

// La couche de projection : l'espace, et une vue par oeil. Rien ne survit a
// l'appel : on la remplit sur la pile, la session la consomme, on l'oublie.
struct CoucheProjection {
    Espace espace = Espace::STAGE;
    Vue    vues[2];             // 0 = oeil gauche, 1 = oeil droit
};
