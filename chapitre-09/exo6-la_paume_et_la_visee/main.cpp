#include <iostream>
#include <string>

int main() {
    int n = 0;
    std::cin >> n;
    int fautes = 0;
    for (int i = 0; i < n; ++i) {
        std::string nom, intention, pose;
        std::cin >> nom >> intention >> pose;
        const char *verdict = "OK";
        if (intention == "TENIR" && pose == "AIM") verdict = "FLOTTE";
        else if (intention == "POINTER" && pose == "GRIP") verdict = "PART DE TRAVERS";
        if (verdict[0] != 'O') ++fautes;
        std::cout << nom << ' ' << verdict << '\n';
    }
    std::cout << "FAUTES " << fautes << '\n';
    return 0;
}
