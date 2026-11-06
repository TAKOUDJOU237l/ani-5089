#include <iostream>
#include <map>
#include <string>

int main() {
    long long images = 0;
    int n = 0;
    std::cin >> images >> n;
    std::map<long long, unsigned long long> poignees;
    for (int i = 0; i < n; ++i) {
        std::string cmd;
        long long index = 0;
        std::cin >> cmd >> index;
        const bool dedans = index >= 0 && index < images;
        if (cmd == "POSE") {
            unsigned long long valeur = 0;
            std::cin >> valeur;
            if (!dedans) std::cout << "HORS ANNEAU\n";
            else poignees[index] = valeur;
        } else {
            auto it = dedans ? poignees.find(index) : poignees.end();
            if (it == poignees.end()) std::cout << "VIDE\n";
            else std::cout << it->second << '\n';
        }
    }
    std::cout << "POSEES " << poignees.size() << '\n';
    return 0;
}
