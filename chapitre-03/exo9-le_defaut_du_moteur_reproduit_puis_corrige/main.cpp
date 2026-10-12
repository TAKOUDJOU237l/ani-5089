#include "NKWindow/NkWindow.h"
#include "NKWindow/Core/NkMain.h"
#include <cstdio>

int nkmain(const nkentseu::NkEntryState& /*state*/)
{
    using namespace nkentseu;

    NkAppData app;
    app.appName           = "Ma salle";
    app.preferredRenderer = NkRendererApi::NK_SOFTWARE;

    if (!NkInitialise(app))
        return 1;

    NkWindowConfig config;
    config.title  = "Ma salle";
    config.width  = 1280;
    config.height = 720;

    Window fenetre(config);
    if (!fenetre.IsOpen())
        return 2;

    auto& evenements = EventSystem::Instance();
    int nbTotal = 0, nbMove = 0, nbRaw = 0;

    while (fenetre.IsOpen())
    {
        while (NkEvent* ev = evenements.PollEvent())
        {
            ++nbTotal;
            if (ev->As<NkWindowCloseEvent>())
                fenetre.Close();
            if (ev->As<NkMouseMoveEvent>())
                ++nbMove;
            if (ev->As<NkMouseRawEvent>())
                ++nbRaw;
        }
    }

    FILE* fc = std::fopen("compteurs.txt", "w");
    if (fc)
    {
        std::fprintf(fc, "total=%d move=%d raw=%d\n", nbTotal, nbMove, nbRaw);
        std::fclose(fc);
    }
    return 0;
}
