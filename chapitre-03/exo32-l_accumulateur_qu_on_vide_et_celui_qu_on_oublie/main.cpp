#include <iostream>
#include <string>

int main() {
    long long n = 0;
    if (!(std::cin >> n)) {
        return 0;
    }

    long long totalX = 0;
    long long totalY = 0;
    long long moteurX = 0;
    long long moteurY = 0;

    for (long long i = 0; i < n; ++i) {
        std::string commande;
        if (!(std::cin >> commande)) {
            break;
        }

        if (commande == "bouge") {
            long long dx = 0;
            long long dy = 0;
            if (!(std::cin >> dx >> dy)) {
                break;
            }
            totalX += dx;
            totalY += dy;
            moteurX = dx;
            moteurY = dy;
        } else if (commande == "image") {
            std::cout << totalX << " " << totalY << " "
                      << moteurX << " " << moteurY << "\n";
            totalX = 0;
            totalY = 0;
        }
    }

    return 0;
}