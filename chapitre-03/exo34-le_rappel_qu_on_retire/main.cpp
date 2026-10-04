#include <algorithm>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

int main() {
    int n = 0;
    std::cin >> n;

    std::vector<std::pair<long long, std::string>> registre;

    for (int i = 0; i < n; ++i) {
        std::string commande;
        std::cin >> commande;

        if (commande == "poser") {
            long long id = 0;
            std::string type;
            std::cin >> id >> type;
            registre.erase(
                std::remove_if(registre.begin(), registre.end(),
                               [id](const std::pair<long long, std::string> &r) {
                                   return r.first == id;
                               }),
                registre.end());
            registre.push_back({id, type});
        } else if (commande == "retirer") {
            long long id = 0;
            std::cin >> id;
            registre.erase(
                std::remove_if(registre.begin(), registre.end(),
                               [id](const std::pair<long long, std::string> &r) {
                                   return r.first == id;
                               }),
                registre.end());
        } else if (commande == "envoyer") {
            std::string type;
            std::cin >> type;
            bool premier = true;
            for (const auto &r : registre) {
                if (r.second != type) {
                    continue;
                }
                if (!premier) {
                    std::cout << " ";
                }
                std::cout << r.first;
                premier = false;
            }
            if (premier) {
                std::cout << "AUCUN";
            }
            std::cout << "\n";
        }
    }

    return 0;
}
