#include "NKWindow/NkWindow.h"
#include "NKWindow/Core/NkMain.h"

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

    while (fenetre.IsOpen())
    {
       
        while (NkEvent* ev = evenements.PollEvent())
        {
            if (ev->As<NkWindowCloseEvent>())
                fenetre.Close();
        }
    }

    return 0;
}

