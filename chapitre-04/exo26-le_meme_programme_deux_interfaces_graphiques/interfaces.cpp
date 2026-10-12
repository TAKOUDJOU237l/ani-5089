#include <chrono>

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

static const float INTERIEUR = 4.0f;
static const float HAUTEUR = 2.50f;
static const float EP_MUR = 0.10f;

static const float EP_PANNEAU = 0.02f;
static const float AVANCE = 0.002f;
static const float PORTE_X = -0.70f;
static const float PORTE_L = 0.90f;
static const float PORTE_H = 2.00f;
static const float FENETRE_L = 1.20f;
static const float FENETRE_H = 1.00f;
static const float FENETRE_Y = 1.40f;
static const float FENETRE_Z1 = -0.90f;
static const float FENETRE_Z2 = 0.90f;

static const float TABLE_X = 0.60f;
static const float TABLE_Z = 0.70f;
static const float TABLE_LONGUEUR = 1.20f;
static const float TABLE_PROFONDEUR = 0.80f;
static const float TABLE_DESSUS = 0.75f;
static const float TABLE_EPAISSEUR = 0.05f;
static const float PIED_SECTION = 0.06f;

static const int IMAGES_CHAUFFE = 20;
static const int IMAGES_MESURE = 400;

static bool g_running = true;
static uint32 g_W = 1280;
static uint32 g_H = 720;
static NkRenderer *g_renderer = nullptr;

static void ConfigureAppData(NkAppData &d) {
	d.appName = "Interfaces";
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

	Soumettre(r3d, cube, PoserAuSol({large, HAUTEUR, EP_MUR}, 0.f, murZ), mur);
	Soumettre(r3d, cube, PoserAuSol({large, HAUTEUR, EP_MUR}, 0.f, -murZ), mur);
	Soumettre(r3d, cube, PoserAuSol({EP_MUR, HAUTEUR, INTERIEUR}, murZ, 0.f), mur);
	Soumettre(r3d, cube, PoserAuSol({EP_MUR, HAUTEUR, INTERIEUR}, -murZ, 0.f), mur);

	Soumettre(r3d, cube, NkMat4f::Translate({0.f, -EP_MUR * 0.5f, 0.f}) * NkMat4f::Scale({large, EP_MUR, large}), sol);

	Soumettre(r3d, cube, PoserAuSol({PORTE_L, PORTE_H, EP_PANNEAU}, PORTE_X, faceZ - EP_PANNEAU * 0.5f + AVANCE), porte);

	const float fenetreX = faceX - EP_PANNEAU * 0.5f + AVANCE;
	Soumettre(r3d, cube, NkMat4f::Translate({fenetreX, FENETRE_Y, FENETRE_Z1}) * NkMat4f::Scale({EP_PANNEAU, FENETRE_H, FENETRE_L}), fenetre);
	Soumettre(r3d, cube, NkMat4f::Translate({fenetreX, FENETRE_Y, FENETRE_Z2}) * NkMat4f::Scale({EP_PANNEAU, FENETRE_H, FENETRE_L}), fenetre);

	Soumettre(r3d, cube, NkMat4f::Translate({TABLE_X, TABLE_DESSUS - TABLE_EPAISSEUR * 0.5f, TABLE_Z}) * NkMat4f::Scale({TABLE_LONGUEUR, TABLE_EPAISSEUR, TABLE_PROFONDEUR}), bois);
	const float dx = TABLE_LONGUEUR * 0.5f - PIED_SECTION * 0.5f;
	const float dz = TABLE_PROFONDEUR * 0.5f - PIED_SECTION * 0.5f;
	const float piedH = TABLE_DESSUS - TABLE_EPAISSEUR;
	for (int sx = -1; sx <= 1; sx += 2)
		for (int sz = -1; sz <= 1; sz += 2)
			Soumettre(r3d, cube, PoserAuSol({PIED_SECTION, piedH, PIED_SECTION}, TABLE_X + sx * dx, TABLE_Z + sz * dz), pied);

	Soumettre(r3d, cube, PoserSurTable({0.30f, 0.04f, 0.21f}, 0.40f, 0.60f), fenetre);
	Soumettre(r3d, cube, PoserSurTable({0.08f, 0.12f, 0.08f}, 0.85f, 0.80f), porte);
}

static double Lancer(NkGraphicsApi api) {
	g_running = true;

	NkWindowConfig winCfg;
	winCfg.title = "Interfaces";
	winCfg.width = 1280;
	winCfg.height = 720;
	winCfg.centered = true;
	winCfg.resizable = true;

	NkWindow window(winCfg);
	if (!window.IsValid()) {
		logger.Error("[Interfaces] refus : creation fenetre KO");
		return -1.0;
	}

	NkDeviceInitInfo devInfo{};
	devInfo.surface = window.GetSurfaceDesc();
	devInfo.width = (uint32)window.GetSize().width;
	devInfo.height = (uint32)window.GetSize().height;
	g_W = devInfo.width;
	g_H = devInfo.height;

	NkIDevice *device = NkDeviceFactory::CreateForApi(api, devInfo);
	if (!device || !device->IsValid()) {
		logger.Error("[Interfaces] refus : {0} ne demarre pas", NkGraphicsApiName(api));
		if (device)
			NkDeviceFactory::Destroy(device);
		window.Close();
		return -1.0;
	}
	logger.Info("[Interfaces] Backend : {0}", NkGraphicsApiName(api));

	NkRendererConfig cfg = NkRendererConfig::ForGame(api, g_W, g_H);
	cfg.api = api;
	cfg.subsystems = NK_SS_RENDER3D | NK_SS_SHADOW;

	NkRenderer *renderer = NkRenderer::Create(device, cfg);
	if (!renderer || !renderer->Initialize()) {
		logger.Error("[Interfaces] refus : init renderer KO ({0})", NkGraphicsApiName(api));
		NkRenderer::Destroy(renderer);
		NkDeviceFactory::Destroy(device);
		window.Close();
		return -1.0;
	}
	g_renderer = renderer;

	NkRender3D *r3d = renderer->GetRender3D();
	NkMeshHandle cube = renderer->GetMeshSystem()->GetCube();

	const NkVec3f pos{1.5f, 1.5f, 1.7f};
	const NkVec3f cible{-0.3f, 0.8f, -0.3f};

	NkEventSystem &events = NkEvents();
	int frame = 0;
	double moyenneMs = -1.0;
	auto t0 = std::chrono::steady_clock::now();

	while (g_running && window.IsOpen()) {
		events.PollEvents();
		if (!g_running)
			break;
		if (!renderer->BeginFrame())
			continue;

		NkCamera3DData camData;
		camData.up = {0.f, 1.f, 0.f};
		camData.fovY = 70.f;
		camData.aspect = (float32)g_W / (float32)g_H;
		camData.nearPlane = 0.05f;
		camData.farPlane = 200.f;
		NkCamera3D cam(camData);
		cam.SetPosition(pos);
		cam.SetTarget(cible);

		NkSceneContext sctx;
		sctx.camera = cam;

		NkLightDesc soleil;
		soleil.type = NkLightType::NK_DIRECTIONAL;
		soleil.direction = {-0.4f, -1.f, -0.3f};
		soleil.color = {1.f, 0.95f, 0.85f};
		soleil.intensity = 3.f;
		soleil.castShadow = true;
		sctx.lights.PushBack(soleil);

		r3d->BeginScene(sctx);
		ConstruireSalle(r3d, cube);
		renderer->Present();
		renderer->EndFrame();

		++frame;
		if (frame == IMAGES_CHAUFFE)
			t0 = std::chrono::steady_clock::now();
		if (frame == IMAGES_CHAUFFE + IMAGES_MESURE) {
			std::chrono::duration<double, std::milli> dt = std::chrono::steady_clock::now() - t0;
			moyenneMs = dt.count() / IMAGES_MESURE;
			logger.Info("[Interfaces] {0} : {1} ms/image (moyenne sur {2} images). Capture, puis Echap.",
			            NkGraphicsApiName(api), moyenneMs, IMAGES_MESURE);
		}
	}

	device->WaitIdle();
	g_renderer = nullptr;
	NkRenderer::Destroy(renderer);
	NkDeviceFactory::Destroy(device);
	window.Close();
	return moyenneMs;
}

int nkmain(const NkEntryState &state) {
	(void)state;

	NkEventSystem &events = NkEvents();
	events.AddEventCallback<NkWindowCloseEvent>([](NkWindowCloseEvent *) { g_running = false; });
	events.AddEventCallback<NkKeyPressEvent>([](NkKeyPressEvent *e) {
		if (e->GetKey() == NkKey::NK_ESCAPE)
			g_running = false;
	});
	events.AddEventCallback<NkWindowResizeEvent>([](NkWindowResizeEvent *e) {
		const uint32 w = (uint32)e->GetWidth(), h = (uint32)e->GetHeight();
		if (w > 0 && h > 0 && (w != g_W || h != g_H)) {
			g_W = w;
			g_H = h;
			if (g_renderer)
				g_renderer->OnResize(w, h);
		}
	});

	Lancer(NkGraphicsApi::NK_GFX_API_SOFTWARE);
	Lancer(NkGraphicsApi::NK_GFX_API_OPENGL);
	return 0;
}