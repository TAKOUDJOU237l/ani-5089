#include <iostream>
#include <map>
#include <string>

int main() {
    int a = 0;
    std::cin >> a;
    std::map<std::string, std::string> types;
    for (int i = 0; i < a; ++i) {
        std::string nom, type;
        std::cin >> nom >> type;
        types[nom] = type;
    }
    int o = 0;
    std::cin >> o;
    for (int i = 0; i < o; ++i) {
        std::string nom, op;
        std::cin >> nom >> op;
        auto it = types.find(nom);
        if (it == types.end()) continue;
        const bool sortie = it->second == "HAPTIC";
        if (op == "LIT" && sortie) std::cout << nom << " LIT UNE SORTIE\n";
        else if (op == "ECRIT" && !sortie) std::cout << nom << " ECRIT UNE ENTREE\n";
    }
    int entrees = 0, sorties = 0;
    for (const auto &p : types) {
        if (p.second == "HAPTIC") ++sorties; else ++entrees;
    }
    std::cout << "ENTREES " << entrees << '\n';
    std::cout << "SORTIES " << sorties << '\n';
    return 0;
}
