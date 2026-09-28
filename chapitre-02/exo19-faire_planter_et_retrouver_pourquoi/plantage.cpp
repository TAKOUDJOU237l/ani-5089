#include <cstdio>

int main() {
    printf("Avant le plantage\n");

    int *pointeurNul = nullptr;
    *pointeurNul = 42; // déréférencement d'un pointeur nul : plantage volontaire

    printf("Cette ligne ne s'affichera jamais\n");
    return 0;
}
