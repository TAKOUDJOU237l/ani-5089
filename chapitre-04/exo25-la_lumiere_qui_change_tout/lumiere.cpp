#include "NKPlatform/NkPlatformDetect.h"
#include "NKWindow/NKMain.h"
#include "NKWindow/Core/NkWindow.h"
#include "NKWindow/Core/NkWindowConfig.h"
#include "NKEvent/NkWindowEvent.h"
#include "NKEvent/NkKeyboardEvent.h"
#include "NKLogger/NkLog.h"

#include "NKRHI/Core/NkDeviceFactory.h"
#include "NKRenderer/NkRenderer.h"
#include "NKRenderer/Core/NkCamera.h"
#include "NKRenderer/Mesh/NkMeshSystem.h"
#include "NKRenderer/Tools/Render3D/NkRender3D.h"

using namespace nkentseu;
using namespace nkentseu::renderer;

// Un seul réglage change à la fois :
// 0 = départ, 1 = direction changée, 2 = intensité changée, 3 = ombres coupées
#define REGLAGE 0


static const NkVec3f SOLEIL_DIRECTION_DEPART{-0.4f, -1.f, -0.3f};
static const NkVec3f SOLEIL_DIRECTION_ESSAI{0.6f, -1.f, 0.2f}; 
static const float SOLEIL_INTENSITE_DEPART = 3.f;
static const float SOLEIL_INTENSITE_ESSAI = 0.8f;


static const float INTERIEUR = 4.0f;
static const float HAUTEUR = 2.50f;
static const float EP_MUR = 0.10f;

static const float EP_PANNEAU = 0.02f;
static const float PORTE_X = -0.70f;
static const float PORTE_L = 0.90f;
static const float PORTE_H = 2.00f;
static const float FENETRE_L = 1.20f;
static const float FENETRE_H = 1.00f;
static const float FENETRE_Y = 1.40f;
static const float FENETRE_Z1 = -0.90f;
static const float FENETRE_Z2 = 0.90f;
static const float AVANCE = 0.002f; 

static const float TABLE_X = 0.60f;
static const float TABLE_Z = 0.70f;
static const float TABLE_LONGUEUR = 1.20f;
static const float TABLE_PROFONDEUR = 0.80f;
static const float TABLE_DESSUS = 0.75f; 
static const float TABLE_EPAISSEUR = 0.05f;
static const float PIED_SECTION = 0.06f;

static void ConfigureAppData(NkAppData &d) {
	d.appName = "Lumiere";
}
NK_REGISTER_ENTRY_APPDATA_UPDATER(ConfigureAppData)


NkMat4f PoserAuSol(NkVec3f taille, float x, float z) {
	return NkMat4f::Translate({x, taille.y * 0.5f, z}) * NkMat4f::Scale(taille);
}


NkMat4f PoserSurTable(NkVec3f taille, float x, float z) {
	return NkMat4f::Translate({x, TABLE_DESSUS + taille.y * 0.5f, z}) * NkMat4f::Scale(taille);
}


static void Soumettre(NkRender3D *r3d, NkMeshHandle cube, const NkMat4f &m, NkVec3f couleur) {
	NkDrawCall3D d;
	d.mesh = cube;
	d.transform = m;
	d.tint = couleur;
	d.metallic = 0.f;
	d.roughness = 0.9f;
	d.aabb = NkAABB::Unit().Transformed(m);
	r3d->Submit(d);
}

static void ConstruireSalle(NkRender3D *r3d, NkMeshHandle cube) {
	const NkVec3f mur{0.72f, 0.70f, 0.66f}, sol{0.35f, 0.33f, 0.30f};
	const NkVec3f porte{0.45f, 0.28f, 0.15f}, fenetre{0.55f, 0.75f, 0.90f};
	const NkVec3f bois{0.62f, 0.42f, 0.22f}, pied{0.42f, 0.27f, 0.13f};

	const float large = INTERIEUR + 2.f * EP_MUR;
	const float murZ = -(INTERIEUR * 0.5f + EP_MUR * 0.5f); 
	const float faceZ = murZ + EP_MUR * 0.5f;               
	const float faceX = -(INTERIEUR * 0.5f);               
	// Ce qui repose sur le sol : PoserAuSol.
	Soumettre(r3d, cube, PoserAuSol({large, HAUTEUR, EP_MUR}, 0.f, murZ), mur);
	Soumettre(r3d, cube, PoserAuSol({large, HAUTEUR, EP_MUR}, 0.f, -murZ), mur);
	Soumettre(r3d, cube, PoserAuSol({EP_MUR, HAUTEUR, INTERIEUR}, murZ, 0.f), mur);
	Soumettre(r3d, cube, PoserAuSol({EP_MUR, HAUTEUR, INTERIEUR}, -murZ, 0.f), mur);
	Soumettre(r3d, cube, PoserAuSol({PORTE_L, PORTE_H, EP_PANNEAU}, PORTE_X, faceZ - EP_PANNEAU * 0.5f + AVANCE), porte);
	const float dx = TABLE_LONGUEUR * 0.5f - PIED_SECTION * 0.5f;
	const float dz = TABLE_PROFONDEUR * 0.5f - PIED_SECTION * 0.5f;
	const float piedH = TABLE_DESSUS - TABLE_EPAISSEUR;
	for (int sx = -1; sx <= 1; sx += 2)
		for (int sz = -1; sz <= 1; sz += 2)
			Soumettre(r3d, cube, PoserAuSol({PIED_SECTION, piedH, PIED_SECTION}, TABLE_X + sx * dx, TABLE_Z + sz * dz), pied);

	
	Soumettre(r3d, cube, PoserSurTable({0.30f, 0.04f, 0.21f}, 0.40f, 0.60f), fenetre);  
	Soumettre(r3d, cube, PoserSurTable({0.08f, 0.12f, 0.08f}, 0.85f, 0.80f), porte);    

	.
	Soumettre(r3d, cube, NkMat4f::Translate({0.f, -EP_MUR * 0.5f, 0.f}) * NkMat4f::Scale({large, EP_MUR, large}), sol);
	
	const float fenetreX = faceX - EP_PANNEAU * 0.5f + AVANCE;
	Soumettre(r3d, cube, NkMat4f::Translate({fenetreX, FENETRE_Y, FENETRE_Z1}) * NkMat4f::Scale({EP_PANNEAU, FENETRE_H, FENETRE_L}), fenetre);
	Soumettre(r3d, cube, NkMat4f::Translate({fenetreX, FENETRE_Y, FENETRE_Z2}) * NkMat4f::Scale({EP_PANNEAU, FENETRE_H, FENETRE_L}), fenetre);
	Soumettre(r3d, cube, NkMat4f::Translate({TABLE_X, TABLE_DESSUS - TABLE_EPAISSEUR * 0.5f, TABLE_Z}) * NkMat4f::Scale({TABLE_LONGUEUR, TABLE_EPAISSEUR, TABLE_PROFONDEUR}), bois);
}

int nkmain(const NkEntryState &state) {
	(void)state;

	NkWindowConfig winCfg;
	winCfg.title = "Lumiere";
	winCfg.width = 1280;
	winCfg.height = 720;
	winCfg.centered = true;
	winCfg.resizable = true;

	NkWindow window(winCfg);
	if (!window.IsValid()) {
		logger.Error("[Lumiere] Creation fenetre KO");
		return 1;
	}

	NkDeviceInitInfo devInfo{};
	devInfo.surface = window.GetSurfaceDesc();
	devInfo.width = (uint32)window.GetSize().width;
	devInfo.height = (uint32)window.GetSize().height;

	NkIDevice *device = NkDeviceFactory::CreateAutoDetect(devInfo);
	if (!device || !device->IsValid()) {
		logger.Error("[Lumiere] Creation device KO");
		window.Close();
		return 2;
	}
	logger.Info("[Lumiere] Backend : {0}", NkGraphicsApiName(device->GetApi()));

	NkRendererConfig cfg = NkRendererConfig::ForGame(devInfo.api, devInfo.width, devInfo.height);
	cfg.subsystems = NK_SS_RENDER3D | NK_SS_SHADOW; 
	NkRenderer *renderer = NkRenderer::Create(device, cfg);
	if (!renderer || !renderer->Initialize()) {
		logger.Error("[Lumiere] Init renderer KO");
		NkRenderer::Destroy(renderer);
		NkDeviceFactory::Destroy(device);
		window.Close();
		return 3;
	}
	NkRender3D *r3d = renderer->GetRender3D();
	NkMeshHandle cube = renderer->GetMeshSystem()->GetCube();

	bool running = true;
	uint32 W = devInfo.width;
	uint32 H = devInfo.height;
	NkEventSystem &events = NkEvents();
	events.AddEventCallback<NkWindowCloseEvent>([&](NkWindowCloseEvent *) { running = false; });
	events.AddEventCallback<NkKeyPressEvent>([&](NkKeyPressEvent *e) {
		if (e->GetKey() == NkKey::NK_ESCAPE)
			running = false;
	});
	events.AddEventCallback<NkWindowResizeEvent>([&](NkWindowResizeEvent *e) {
		const uint32 w = (uint32)e->GetWidth(), h = (uint32)e->GetHeight();
		if (w > 0 && h > 0 && (w != W || h != H)) {
			W = w;
			H = h;
			renderer->OnResize(w, h);
		}
	});

	
	const NkVec3f pos{1.5f, 1.5f, 1.7f};
	const NkVec3f cible{-0.3f, 0.8f, -0.3f};

	while (running && window.IsOpen()) {
		events.PollEvents();
		if (!running)
			break;
		if (!renderer->BeginFrame())
			continue;

		NkCamera3DData camData;
		camData.up = {0.f, 1.f, 0.f};
		camData.fovY = 70.f;
		camData.aspect = (float32)W / (float32)H;
		camData.nearPlane = 0.05f;
		camData.farPlane = 200.f;
		NkCamera3D cam(camData);
		cam.SetPosition(pos);
		cam.SetTarget(cible);

		NkSceneContext sctx;
		sctx.camera = cam;

		
		NkLightDesc soleil;
		soleil.type = NkLightType::NK_DIRECTIONAL;
		soleil.direction = SOLEIL_DIRECTION_DEPART;
		soleil.color = {1.f, 0.95f, 0.85f};
		soleil.intensity = SOLEIL_INTENSITE_DEPART;
		soleil.castShadow = true;
#if REGLAGE == 1
		soleil.direction = SOLEIL_DIRECTION_ESSAI;
#elif REGLAGE == 2
		soleil.intensity = SOLEIL_INTENSITE_ESSAI;
#elif REGLAGE == 3
		soleil.castShadow = false;
#endif
		sctx.lights.PushBack(soleil);

		r3d->BeginScene(sctx);
		ConstruireSalle(r3d, cube);

		renderer->Present();
		renderer->EndFrame();
	}

	device->WaitIdle();
	NkRenderer::Destroy(renderer);
	NkDeviceFactory::Destroy(device);
	window.Close();
	return 0;
}
