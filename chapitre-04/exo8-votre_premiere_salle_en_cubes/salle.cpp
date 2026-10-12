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
static const float EP = 0.10f;

static void ConfigureAppData(NkAppData &d) {
	d.appName = "Salle";
}
NK_REGISTER_ENTRY_APPDATA_UPDATER(ConfigureAppData)

static void Soumettre(NkRender3D *r3d, NkMeshHandle mesh, const NkMat4f &m, NkVec3f couleur) {
	NkDrawCall3D d;
	d.mesh = mesh;
	d.transform = m;
	d.tint = couleur;
	d.metallic = 0.f;
	d.roughness = 0.9f;
	d.aabb = NkAABB::Unit().Transformed(m);
	r3d->Submit(d);
}

static void ConstruireSalle(NkRender3D *r3d, NkMeshHandle cube, NkMeshHandle plan) {
	const NkVec3f mur{0.72f, 0.70f, 0.66f};
	const NkVec3f sol{0.35f, 0.33f, 0.30f};
	const NkVec3f plafond{0.85f, 0.85f, 0.82f};

	const float large = INTERIEUR + 2.f * EP;
	const float decal = INTERIEUR * 0.5f + EP * 0.5f;
	const float milieu = HAUTEUR * 0.5f;

	Soumettre(r3d, plan, NkMat4f::Translate({0.f, 0.f, 0.f}) * NkMat4f::Scale({INTERIEUR, 1.f, INTERIEUR}), sol);

	Soumettre(r3d, cube, NkMat4f::Translate({0.f, milieu, -decal}) * NkMat4f::Scale({large, HAUTEUR, EP}), mur);
	Soumettre(r3d, cube, NkMat4f::Translate({0.f, milieu, decal}) * NkMat4f::Scale({large, HAUTEUR, EP}), mur);
	Soumettre(r3d, cube, NkMat4f::Translate({-decal, milieu, 0.f}) * NkMat4f::Scale({EP, HAUTEUR, INTERIEUR}), mur);
	Soumettre(r3d, cube, NkMat4f::Translate({decal, milieu, 0.f}) * NkMat4f::Scale({EP, HAUTEUR, INTERIEUR}), mur);

	Soumettre(r3d, cube, NkMat4f::Translate({0.f, HAUTEUR + EP * 0.5f, 0.f}) * NkMat4f::Scale({large, EP, large}), plafond);
}

int nkmain(const NkEntryState &state) {
	(void)state;

	NkWindowConfig winCfg;
	winCfg.title = "Salle";
	winCfg.width = 1280;
	winCfg.height = 720;
	winCfg.centered = true;
	winCfg.resizable = true;

	NkWindow window(winCfg);
	if (!window.IsValid()) {
		logger.Error("[Salle] Creation fenetre KO");
		return 1;
	}

	NkDeviceInitInfo devInfo{};
	devInfo.surface = window.GetSurfaceDesc();
	devInfo.width = (uint32)window.GetSize().width;
	devInfo.height = (uint32)window.GetSize().height;

	NkIDevice *device = NkDeviceFactory::CreateAutoDetect(devInfo);
	if (!device || !device->IsValid()) {
		logger.Error("[Salle] Creation device KO");
		window.Close();
		return 2;
	}
	logger.Info("[Salle] Backend : {0}", NkGraphicsApiName(device->GetApi()));

	NkRendererConfig cfg = NkRendererConfig::ForGame(device->GetApi(), devInfo.width, devInfo.height);
	cfg.subsystems = NK_SS_RENDER3D;
	NkRenderer *renderer = NkRenderer::Create(device, cfg);
	if (!renderer || !renderer->Initialize()) {
		logger.Error("[Salle] Init renderer KO");
		NkRenderer::Destroy(renderer);
		NkDeviceFactory::Destroy(device);
		window.Close();
		return 3;
	}
	NkRender3D *r3d = renderer->GetRender3D();
	NkMeshHandle cube = renderer->GetMeshSystem()->GetCube();
	NkMeshHandle plan = renderer->GetMeshSystem()->GetPlane();

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

	const NkVec3f pos{1.8f, 1.70f, 1.8f};
    const NkVec3f cible{-0.5f, 1.2f, -0.5f};
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
		soleil.direction = {-0.4f, -0.5f, -0.3f};
		soleil.color = {1.f, 0.95f, 0.85f};
		soleil.intensity = 3.f;
		soleil.castShadow = false;
		sctx.lights.PushBack(soleil);

		r3d->BeginScene(sctx);
		ConstruireSalle(r3d, cube, plan);
		renderer->Present();
		renderer->EndFrame();
	}

	device->WaitIdle();
	NkRenderer::Destroy(renderer);
	NkDeviceFactory::Destroy(device);
	window.Close();
	return 0;
}