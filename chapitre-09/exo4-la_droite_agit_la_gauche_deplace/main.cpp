#include <iostream>
#include <set>
#include <sstream>
#include <string>

static bool Termine(const std::string &s, const std::string &fin) {
    return s.size() >= fin.size() && s.compare(s.size() - fin.size(), fin.size(), fin) == 0;
}

int main() {
    const std::set<std::string> connus = {
        "SELECT", "GRAB", "MENU", "MOVE", "AIM_POSE", "GRIP_POSE", "HAPTIC",
        "SELECT_LEFT", "GRAB_LEFT", "AIM_POSE_LEFT", "GRIP_POSE_LEFT", "HAPTIC_LEFT",
        "MOVE_RIGHT"};
    std::string ligne;
    std::getline(std::cin, ligne);
    const int n = std::stoi(ligne);
    int deuxMains = 0;
    for (int i = 0; i < n; ++i) {
        std::getline(std::cin, ligne);
        std::istringstream flux(ligne);
        std::string nom, usage;
        flux >> nom;
        int droite = 0, gauche = 0;
        while (flux >> usage) {
            if (connus.count(usage) == 0) continue;
            if (Termine(usage, "_LEFT") || usage == "MENU" || usage == "MOVE") ++gauche;
            else ++droite;
        }
        const bool deux = droite > 0 && gauche > 0;
        if (deux) ++deuxMains;
        std::cout << nom << ' ' << droite << ' ' << gauche << ' '
                  << (deux ? "DEUX MAINS" : "UNE MAIN") << '\n';
    }
    std::cout << "DEUX MAINS " << deuxMains << '\n';
    return 0;
}
