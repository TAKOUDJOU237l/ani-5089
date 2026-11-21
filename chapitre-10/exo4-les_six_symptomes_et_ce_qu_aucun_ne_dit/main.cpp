// Exercice 4 : les six symptomes, et ce qu'aucun ne dit.
#include <cstdio>
#include <string>

static const char *Cause(const std::string &s) {
    if (s == "BLOQUE_AU_WAIT") return "FIN DE TRAME";
    if (s == "PAS_DE_REPRISE") return "FIN SANS RENDU";
    if (s == "OEIL_NOIR") return "CHAINE MANQUANTE";
    if (s == "TREMBLEMENT") return "MAUVAIS INSTANT";
    if (s == "CORRECTION_DE_TRAVERS") return "POSE FRAICHE";
    if (s == "SCENE_DECALEE") return "MAUVAIS ESPACE";
    return nullptr;
}

int main() {
    int n = 0;
    if (std::scanf("%d", &n) != 1) return 0;
    int connus = 0, inconnus = 0;
    for (int i = 0; i < n; ++i) {
        char buf[128];
        if (std::scanf("%127s", buf) != 1) break;
        const char *cause = Cause(buf);
        if (cause != nullptr) { ++connus; std::printf("%s %s\n", buf, cause); }
        else { ++inconnus; std::printf("%s INCONNU\n", buf); }
    }
    std::printf("DIAGNOSTIQUES %d\n", connus);
    std::printf("INCONNUS %d\n", inconnus);
    return 0;
}
