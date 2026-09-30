#include <cstdio>

int main() {
    printf("Avant le plantage\n");
    fflush(stdout);

    // volatile empeche le compilateur de supprimer l'ecriture
    volatile int *pointeurNul = nullptr;
    *pointeurNul = 42; // dereferencement d'un pointeur nul : plantage volontaire

    printf("Cette ligne ne s'affichera jamais\n");
    return 0;
}
