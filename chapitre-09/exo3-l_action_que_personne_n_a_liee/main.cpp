#include <iostream>
#include <set>
#include <string>
#include <vector>

int main() {
    int a = 0;
    std::cin >> a;
    std::vector<std::string> noms(a), usages(a);
    for (int i = 0; i < a; ++i) std::cin >> noms[i] >> usages[i];
    int l = 0;
    std::cin >> l;
    std::set<std::string> fournis;
    for (int i = 0; i < l; ++i) {
        std::string u;
        std::cin >> u;
        fournis.insert(u);
    }
    int mortes = 0;
    for (int i = 0; i < a; ++i) {
        const bool active = fournis.count(usages[i]) > 0;
        if (!active) ++mortes;
        std::cout << noms[i] << ' ' << (active ? "ACTIVE" : "MORTE") << '\n';
    }
    std::cout << "MORTES " << mortes << '\n';
    return 0;
}
