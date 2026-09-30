#include <cstdio>
 
int main() {
    printf("Avant le plantage\n");
    fflush(stdout);
 
    
    volatile int *pointeurNul = nullptr;
    *pointeurNul = 42; 
    printf("Cette ligne ne s'affichera jamais\n");

    return 0;
}

