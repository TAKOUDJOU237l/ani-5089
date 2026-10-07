// Exo 9 : le defaut du mouvement brut, reproduit puis corrige.
//
// Le moteur de cette version (Jenga 2.8.4, exemple 27) n'a pas de fichier
// NkEventState.h : le mouvement brut arrive sous forme d'evenements
// (NkMouseRawEvent, avec GetDeltaX / GetDeltaY), un par message WM_INPUT.
// Il n'existe donc pas de champ du moteur a lire directement. Pour reproduire
// le comportement decrit par l'enonce, ce programme garde dans ses propres
// variables la lecture "naive" : la derniere valeur recue, ecrasee a chaque
// evenement et jamais remise a zero. A cote, il tient l'accumulateur correct.
// Les deux series sont calculees a partir des MEMES evenements, donc de la
// meme manipulation de la souris.
//
// Utilisation : lancer le programme depuis un dossier, faire UN SEUL geste
// rapide et bref de la souris dans la fenetre (un coup de poignet d'environ
// 0,2 seconde), puis ne plus y toucher. Au bout de 6 secondes il ecrit
// avant.txt, apres.txt et serie_complete.txt dans le dossier courant (dix
// images consecutives autour de l'arret du geste), puis il se ferme.

#include "NKWindow/NkWindow.h"
#include "NKWindow/Core/NkMain.h"

#include <chrono>
#include <cstdio>
#include <thread>
#include <vector>

struct Image
{
    int naifX;  // lecture naive : derniere valeur, jamais remise a zero
    int naifY;
    int totalX; // accumulateur : somme de tous les evenements de l'image
    int totalY;
};

int nkmain(const nkentseu::NkEntryState& /*state*/)
{
    using namespace nkentseu;

    NkAppData app;
    app.appName           = "Mouvement brut";
    app.preferredRenderer = NkRendererApi::NK_SOFTWARE;

    if (!NkInitialise(app))
        return 1;

    NkWindowConfig config;
    config.title  = "Mouvement brut";
    config.width  = 1280;
    config.height = 720;

    Window fenetre(config);
    if (!fenetre.IsOpen())
        return 2;

    auto& evenements = EventSystem::Instance();

    int naifX = 0;   // ECRASE a chaque evenement, jamais remis a zero
    int naifY = 0;
    int totalX = 0;  // AJOUTE a chaque evenement, remis a zero a chaque image
    int totalY = 0;

    std::vector<Image> images;
    const auto debut = std::chrono::steady_clock::now();

    while (fenetre.IsOpen())
    {
        while (NkEvent* ev = evenements.PollEvent())
        {
            if (ev->As<NkWindowCloseEvent>())
                fenetre.Close();

            if (auto* brut = ev->As<NkMouseRawEvent>())
            {
                naifX = brut->GetDeltaX();
                naifY = brut->GetDeltaY();
                totalX += brut->GetDeltaX();
                totalY += brut->GetDeltaY();
            }
        }

        // Une fois par image : on lit le total puis on le remet a zero,
        // meme si rien n'a bouge.
        images.push_back({naifX, naifY, totalX, totalY});
        totalX = 0;
        totalY = 0;

        const auto ecoule = std::chrono::steady_clock::now() - debut;
        if (ecoule >= std::chrono::seconds(6))
            fenetre.Close();

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    // Toute la serie, pour pouvoir choisir une autre fenetre de dix images.
    if (FILE* f = std::fopen("serie_complete.txt", "w"))
    {
        std::fprintf(f, "image naifX naifY totalX totalY\n");
        for (size_t i = 0; i < images.size(); ++i)
            std::fprintf(f, "%zu %d %d %d %d\n", i, images[i].naifX,
                         images[i].naifY, images[i].totalX, images[i].totalY);
        std::fclose(f);
    }

    // Derniere image avec un mouvement : on garde dix images autour de l'arret.
    long dernier = -1;
    for (size_t i = 0; i < images.size(); ++i)
    {
        if (images[i].totalX != 0 || images[i].totalY != 0)
            dernier = static_cast<long>(i);
    }

    if (dernier >= 0 && images.size() >= 10)
    {
        long debutFenetre = dernier >= 5 ? dernier - 5 : 0;
        if (debutFenetre + 10 > static_cast<long>(images.size()))
            debutFenetre = static_cast<long>(images.size()) - 10;

        FILE* avant = std::fopen("avant.txt", "w");
        FILE* apres = std::fopen("apres.txt", "w");
        if (avant && apres)
        {
            for (long i = debutFenetre; i < debutFenetre + 10; ++i)
            {
                std::fprintf(avant, "%d %d\n", images[i].naifX, images[i].naifY);
                std::fprintf(apres, "%d %d\n", images[i].totalX, images[i].totalY);
            }
        }
        if (avant) std::fclose(avant);
        if (apres) std::fclose(apres);
    }

    return 0;
}
