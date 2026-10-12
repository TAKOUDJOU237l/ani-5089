
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
static const float HAUTEUR   = 2.50f;  
static const float EP_MUR    = 0.10f;  


static const float EP_PANNEAU = 0.02f; 

static const float PORTE_X = -0.70f;   
static const float PORTE_L = 0.90f;    
static const float PORTE_H = 2.00f;    

static const float FENETRE_L = 1.20f;  
static const float FENETRE_H = 1.00f; 
static const float FENETRE_Y = 1.40f;  
static const float FENETRE_Z1 = -0.90f;
static const float FENETRE_Z2 = 0.90f;


static const float AVANCE = 0.0f;

static void ConfigureAppData(NkAppData &d) {
	d.appName = "Ouvertures";
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
	winCfg.title = "La porte et les fenetres";
	winCfg.width = 1280;
	winCfg.height = 720;
	winCfg.centered = true;
	winCfg.resizable = true;

	NkWindow window(winCfg);
	if (!window.IsValid()) {
		logger.Error("[Ouvertures] Creation fenetre KO");
		return 1;
	}

	NkDeviceInitInfo devInfo{};
	devInfo.surface = window.GetSurfaceDesc();
	devInfo.width = (uint32)window.GetSize().width;
	devInfo.height = (uint32)window.GetSize().height;

	NkIDevice *device = NkDeviceFactory::CreateAutoDetect(devInfo);
	if (!device || !device->IsValid()) {
		logger.Error("[Ouvertures] Creation device KO");
		window.Close();
		return 2;
	}
	logger.Info("[Ouvertures] Backend : {0}", NkGraphicsApiName(device->GetApi()));

	NkRendererConfig cfg = NkRendererConfig::ForGame(devInfo.api, devInfo.width, devInfo.height);
	cfg.subsystems = NK_SS_RENDER3D;
	NkRenderer *renderer = NkRenderer::Create(device, cfg);
	if (!renderer || !renderer->Initialize()) {
		logger.Error("[Ouvertures] Init renderer KO");
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

	
	const float demi = INTERIEUR * 0.5f;                  
	const float murFondZ = -(demi + EP_MUR * 0.5f);       
	const float murGaucheX = -(demi + EP_MUR * 0.5f);     
	
	const float faceFondZ = murFondZ + EP_MUR * 0.5f;    
	const float faceGaucheX = murGaucheX + EP_MUR * 0.5f; 

	
	const float porteZ = faceFondZ - EP_PANNEAU * 0.5f + AVANCE;
	const float fenetreX = faceGaucheX - EP_PANNEAU * 0.5f + AVANCE;

	
	const NkVec3f pos{0.8f, 1.60f, 1.4f};
	const NkVec3f cible{-1.2f, 1.30f, -1.2f};

	const NkVec3f couleurMur{0.72f, 0.70f, 0.66f};
	const NkVec3f couleurSol{0.35f, 0.33f, 0.30f};
	const NkVec3f couleurPorte{0.45f, 0.28f, 0.15f};
	const NkVec3f couleurFenetre{0.55f, 0.75f, 0.90f};

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

		const float large = INTERIEUR + 2.f * EP_MUR; 

		
		Poser(r3d, cube, {0.f, -EP_MUR * 0.5f, 0.f}, {large, EP_MUR, large}, couleurSol);
		Poser(r3d, cube, {0.f, HAUTEUR + EP_MUR * 0.5f, 0.f}, {large, EP_MUR, large}, couleurMur);

		
		Poser(r3d, cube, {0.f, HAUTEUR * 0.5f, murFondZ}, {large, HAUTEUR, EP_MUR}, couleurMur);   // fond (z < 0)
		Poser(r3d, cube, {0.f, HAUTEUR * 0.5f, -murFondZ}, {large, HAUTEUR, EP_MUR}, couleurMur);  // entrée (z > 0)
		Poser(r3d, cube, {murGaucheX, HAUTEUR * 0.5f, 0.f}, {EP_MUR, HAUTEUR, INTERIEUR}, couleurMur);  // gauche (x < 0)
		Poser(r3d, cube, {-murGaucheX, HAUTEUR * 0.5f, 0.f}, {EP_MUR, HAUTEUR, INTERIEUR}, couleurMur); // droite (x > 0)

		
		Poser(r3d, cube, {PORTE_X, PORTE_H * 0.5f, porteZ}, {PORTE_L, PORTE_H, EP_PANNEAU}, couleurPorte);

		
		Poser(r3d, cube, {fenetreX, FENETRE_Y, FENETRE_Z1}, {EP_PANNEAU, FENETRE_H, FENETRE_L}, couleurFenetre);
		Poser(r3d, cube, {fenetreX, FENETRE_Y, FENETRE_Z2}, {EP_PANNEAU, FENETRE_H, FENETRE_L}, couleurFenetre);

		renderer->Present();
		renderer->EndFrame();
	}

	device->WaitIdle();
	NkRenderer::Destroy(renderer);
	NkDeviceFactory::Destroy(device);
	window.Close();
	return 0;
}