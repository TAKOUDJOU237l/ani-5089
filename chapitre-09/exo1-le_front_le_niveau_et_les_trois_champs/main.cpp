#include <iostream>

int main() {
    int n = 0;
    std::cin >> n;
    int precedent = 0, fronts = 0, niveaux = 0;
    for (int image = 1; image <= n; ++image) {
        int presse = 0, lie = 0;
        std::cin >> presse >> lie;
        const int courant = lie ? presse : 0;
        const int change = courant != precedent ? 1 : 0;
        if (courant && change) ++fronts;
        if (courant) ++niveaux;
        std::cout << image << ' ' << courant << ' ' << change << ' ' << lie << '\n';
        precedent = courant;
    }
    std::cout << "FRONTS " << fronts << '\n';
    std::cout << "NIVEAUX " << niveaux << '\n';
    return 0;
}
