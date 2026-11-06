#include <iostream>
#include <string>

int main() {
    int n = 0;
    std::cin >> n;
    bool acquise = false, attend = false;
    int fautes = 0;
    for (int tour = 1; tour <= n; ++tour) {
        std::string cmd;
        std::cin >> cmd;
        const char *verdict = "OK";
        if (cmd == "ACQUIERT") {
            if (acquise) verdict = "DEJA ACQUISE"; else acquise = true;
        } else if (cmd == "DESSINE") {
            if (!acquise) verdict = "PAS ACQUISE";
        } else if (cmd == "REND") {
            if (!acquise) { verdict = "RIEN ACQUIS"; }
            else { acquise = false; attend = true; }
        } else if (cmd == "SOUMET") {
            if (!attend) verdict = "PAS RENDUE"; else attend = false;
        }
        if (verdict[1] != 'K') ++fautes;
        std::cout << tour << ' ' << cmd << ' ' << verdict << '\n';
    }
    std::cout << "FAUTES " << fautes << '\n';
    return 0;
}
