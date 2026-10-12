
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

#include <cmath>

using namespace nkentseu;
using namespace nkentseu::renderer;


#define PROVOQUER_CAUSE 0

static void ConfigureAppData(NkAppData &d) {
	d.appName = "Cube";
}
NK_REGISTER_ENTRY_APPDATA_UPDATER(ConfigureAppData)

int nkmain(const NkEntryState &state) {
	(void)state;

	// --- Fenêtre ---
	NkWindowConfig winCfg;
	winCfg.title = "Un cube, et rien d'autre";
	winCfg.width = 1280;
	winCfg.height = 720;
	winCfg.centered = true;
	winCfg.resizable = true;

	NkWindow window(winCfg);
	if (!window.IsValid()) {
		logger.Error("[Cube] Creation fenetre KO");
		return 1;
	}

	
	NkDeviceInitInfo devInfo{};
	devInfo.surface = window.GetSurfaceDesc();
	devInfo.width = (uint32)window.GetSize().width;
	devInfo.height = (uint32)window.GetSize().height;

	NkIDevice *device = NkDeviceFactory::CreateAutoDetect(devInfo);
	if (!device || !device->IsValid()) {
		logger.Error("[Cube] Creation device KO");
		window.Close();
		return 2;
	}
	logger.Info("[Cube] Backend : {0}", NkGraphicsApiName(device->GetApi()));

	
	NkRendererConfig cfg = NkRendererConfig::ForGame(devInfo.api, devInfo.width, devInfo.height);
	cfg.subsystems = NK_SS_RENDER3D;
	NkRenderer *renderer = NkRenderer::Create(device, cfg);
	if (!renderer || !renderer->Initialize()) {
		logger.Error("[Cube] Init renderer KO");
		NkRenderer::Destroy(renderer);
		NkDeviceFactory::Destroy(device);
		window.Close();
		return 3;
	}
	NkRender3D *r3d = renderer->GetRender3D();
	NkMeshHandle meshCube = renderer->GetMeshSystem()->GetCube(); 

	
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

	
	const NkVec3f pos{0.f, 1.70f, 1.5f};
	const NkVec3f cible{0.f, 1.40f, -2.f};

	
	const float32 dx = cible.x - pos.x;
	const float32 dy = cible.y - pos.y;
	const float32 dz = cible.z - pos.z;
	const float32 longueur = std::sqrt(dx * dx + dy * dy + dz * dz);
	const float32 DISTANCE = 2.0f;
	const NkVec3f centre{pos.x + dx / longueur * DISTANCE, pos.y + dy / longueur * DISTANCE,
						 pos.z + dz / longueur * DISTANCE};
	const NkVec3f taille{0.5f, 0.5f, 0.5f}; // arête de 0,5 m

	
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
		soleil.direction = {-0.4f, -1.f, -0.3f};
		soleil.color = {1.f, 0.95f, 0.85f};
		soleil.intensity = 3.f;
		soleil.castShadow = false;
		sctx.lights.PushBack(soleil);

		r3d->BeginScene(sctx);

		NkDrawCall3D d;
		d.mesh = meshCube;
		d.transform = NkMat4f::Translate(centre) * NkMat4f::Scale(taille); 
		d.tint = {1.f, 1.f, 1.f};
#if !PROVOQUER_CAUSE
		
		d.aabb.min = {centre.x - taille.x * 0.5f, centre.y - taille.y * 0.5f, centre.z - taille.z * 0.5f};
		d.aabb.max = {centre.x + taille.x * 0.5f, centre.y + taille.y * 0.5f, centre.z + taille.z * 0.5f};
#endif
		r3d->Submit(d);

		renderer->Present();
		renderer->EndFrame();
	}

	
	device->WaitIdle();
	NkRenderer::Destroy(renderer);
	NkDeviceFactory::Destroy(device);
	window.Close();
	return 0;
}