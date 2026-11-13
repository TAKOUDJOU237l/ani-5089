#include <cstdlib>
#include <iostream>

int main() {
    long long seuil = 0, maximum = 0;
    int n = 0;
    std::cin >> seuil >> maximum >> n;
    const long long course = 1000 - seuil;
    int morts = 0;
    for (int i = 0; i < n; ++i) {
        long long brut = 0;
        std::cin >> brut;
        const long long absolu = brut < 0 ? -brut : brut;
        long long vitesse = 0;
        if (absolu < seuil) {
            ++morts;
        } else if (course > 0) {
            const long long reste = absolu - seuil;
            vitesse = maximum * reste * reste / (course * course);
            if (brut < 0) vitesse = -vitesse;
        }
        std::cout << vitesse << '\n';
    }
    std::cout << "MORTS " << morts << '\n';
    return 0;
}
