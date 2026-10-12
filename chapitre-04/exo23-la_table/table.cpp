
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


static const float TABLE_X = 0.60f;       
static const float TABLE_Z = 0.70f;       
static const float LONGUEUR = 1.20f;      
static const float PROFONDEUR = 0.80f;    
static const float DESSUS_Y = 0.75f;      
static const float EPAISSEUR = 0.05f;     
static const float SECTION = 0.06f;       

static void ConfigureAppData(NkAppData &d) {
	d.appName = "Table";
}
NK_REGISTER_ENTRY_APPDATA_UPDATER(ConfigureAppData)


static void Poser(NkRender3D *r3d, NkMeshHandle cube, NkVec3f c, NkVec3f t, NkVec3f couleur) {
	NkDrawCall3D d;
	d.mesh = cube;
	d.transform = NkMat4f::Translate(c) * NkMat4f::Scale(t); 
	d.tint = couleur;
	d.metallic = 0.f;
	d.roughness = 0.9f;
	
	d.aabb.min = {c.x - t.x * 0.5f, c.y - t.y * 0.5f, c.z - t.z * 0.5f};
	d.aabb.max = {c.x + t.x * 0.5f, c.y + t.y * 0.5f, c.z + t.z * 0.5f};
	r3d->Submit(d);
}

int nkmain(const NkEntryState &state) {
	(void)state;

	NkWindowConfig winCfg;
	winCfg.title = "La table";
	winCfg.width = 1280;
	winCfg.height = 720;
	winCfg.centered = true;
	winCfg.resizable = true;

	NkWindow window(winCfg);
	if (!window.IsValid()) {
		logger.Error("[Table] Creation fenetre KO");
		return 1;
	}

	NkDeviceInitInfo devInfo{};
	devInfo.surface = window.GetSurfaceDesc();
	devInfo.width = (uint32)window.GetSize().width;
	devInfo.height = (uint32)window.GetSize().height;

	NkIDevice *device = NkDeviceFactory::CreateAutoDetect(devInfo);
	if (!device || !device->IsValid()) {
		logger.Error("[Table] Creation device KO");
		window.Close();
		return 2;
	}
	logger.Info("[Table] Backend : {0}", NkGraphicsApiName(device->GetApi()));

	NkRendererConfig cfg = NkRendererConfig::ForGame(devInfo.api, devInfo.width, devInfo.height);
	cfg.subsystems = NK_SS_RENDER3D;
	NkRenderer *renderer = NkRenderer::Create(device, cfg);
	if (!renderer || !renderer->Initialize()) {
		logger.Error("[Table] Init renderer KO");
		NkRenderer::Destroy(renderer);
		NkDeviceFactory::Destroy(device);
		window.Close();
		return 3;
	}
	NkRender3D *r3d = renderer->GetRender3D();
	NkMeshHandle cube = renderer->GetMeshSystem()->GetCube(); // unitaire, de -0,5 à +0,5

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

	
	const float plateauY = DESSUS_Y - EPAISSEUR * 0.5f;
	
	const float dessousY = DESSUS_Y - EPAISSEUR;
	const float piedH = dessousY;           
	const float piedY = piedH * 0.5f;       
	const float decalageX = LONGUEUR * 0.5f - SECTION * 0.5f;   
	const float decalageZ = PROFONDEUR * 0.5f - SECTION * 0.5f; 

	const NkVec3f pos{-0.6f, 1.5f, 2.4f};
	const NkVec3f cible{0.6f, 0.6f, 0.7f};

	const NkVec3f couleurSol{0.35f, 0.33f, 0.30f};
	const NkVec3f couleurPlateau{0.62f, 0.42f, 0.22f};
	const NkVec3f couleurPied{0.42f, 0.27f, 0.13f};

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

		
		Poser(r3d, cube, {0.f, -0.05f, 0.f}, {6.f, 0.10f, 6.f}, couleurSol);

		
		Poser(r3d, cube, {TABLE_X, plateauY, TABLE_Z}, {LONGUEUR, EPAISSEUR, PROFONDEUR}, couleurPlateau);

		
		for (int sx = -1; sx <= 1; sx += 2) {
			for (int sz = -1; sz <= 1; sz += 2) {
				Poser(r3d, cube, {TABLE_X + sx * decalageX, piedY, TABLE_Z + sz * decalageZ},
					  {SECTION, piedH, SECTION}, couleurPied);
			}
		}

		renderer->Present();
		renderer->EndFrame();
	}

	device->WaitIdle();
	NkRenderer::Destroy(renderer);
	NkDeviceFactory::Destroy(device);
	window.Close();
	return 0;
}