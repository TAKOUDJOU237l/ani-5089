#include <iostream>
#include <string>
#include <vector>

int main() {
    int c = 0;
    std::cin >> c;

    std::vector<long long> echelles(c);
    std::vector<long long> seuils(c);
    for (int i = 0; i < c; ++i) {
        std::string nom;
        std::cin >> nom >> echelles[i] >> seuils[i];
    }

    int t = 0;
    std::cin >> t;

    for (int tour = 0; tour < t; ++tour) {
        long long axe = 0;
        for (int i = 0; i < c; ++i) {
            long long brute = 0;
            std::cin >> brute;

            // L'échelle s'applique avant le seuil.
            long long contribution = brute * echelles[i] / 1000;
            long long absolue = contribution < 0 ? -contribution : contribution;

            if (absolue < seuils[i]) {
                continue;
            }
            axe += contribution;
        }
        std::cout << axe << "\n";
    }

    return 0;
}
