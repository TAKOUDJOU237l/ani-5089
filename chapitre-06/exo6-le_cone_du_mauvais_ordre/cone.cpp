#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

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
#include "NKRenderer/Core/NkTextureLibrary.h"
#include "NKRenderer/Materials/NkMaterial.h"
#include "NKRenderer/Mesh/NkMeshSystem.h"
#include "NKRenderer/Tools/Render3D/NkRender3D.h"
#include <cmath>
#include "NKEvent/NkMouseEvent.h"
#include "NKMath/NkQuat.h"
#include "NKMath/NkAngle.h"

using namespace nkentseu;
using namespace nkentseu::renderer;

static void ConfigureAppData(NkAppData &d) {
	d.appName = "Cone";
}
NK_REGISTER_ENTRY_APPDATA_UPDATER(ConfigureAppData)

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

static void SoumettreMat(NkRender3D *r3d, NkMeshHandle cube, const NkMat4f &m, NkMaterial *mat) {
	NkDrawCall3D d;
	d.mesh = cube;
	d.transform = m;
	d.tint = {1.f, 1.f, 1.f};
	d.metallic = 0.f;
	d.roughness = 0.9f;
	d.material = mat->GetInstHandle();
	d.aabb = NkAABB::Unit().Transformed(m);
	r3d->Submit(d);
}

static void ConstruireSalle(NkRender3D *r3d, NkMeshHandle cube, NkMaterial *murMat, NkMaterial *panneauMat, float panneauLargeur) {
	const NkVec3f mur{0.72f, 0.70f, 0.66f}, sol{0.35f, 0.33f, 0.30f};
	const NkVec3f porte{0.45f, 0.28f, 0.15f}, fenetre{0.55f, 0.75f, 0.90f};
	const NkVec3f bois{0.62f, 0.42f, 0.22f}, pied{0.42f, 0.27f, 0.13f};

	const float large = INTERIEUR + 2.f * EP_MUR;
	const float murZ = -(INTERIEUR * 0.5f + EP_MUR * 0.5f);
	const float faceZ = murZ + EP_MUR * 0.5f;
	const float faceX = -(INTERIEUR * 0.5f);

	const NkMat4f murFond = PoserAuSol({large, HAUTEUR, EP_MUR}, 0.f, murZ);
	if (murMat)
		SoumettreMat(r3d, cube, murFond, murMat);
	else
		Soumettre(r3d, cube, murFond, mur);
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

	Soumettre(r3d, cube, NkMat4f::Translate({0.f, -EP_MUR * 0.5f, 0.f}) * NkMat4f::Scale({large, EP_MUR, large}), sol);
	Soumettre(r3d, cube, NkMat4f::Translate({0.f, HAUTEUR + EP_MUR * 0.5f, 0.f}) * NkMat4f::Scale({large, EP_MUR, large}), mur);
	const float fenetreX = faceX - EP_PANNEAU * 0.5f + AVANCE;
	Soumettre(r3d, cube, NkMat4f::Translate({fenetreX, FENETRE_Y, FENETRE_Z1}) * NkMat4f::Scale({EP_PANNEAU, FENETRE_H, FENETRE_L}), fenetre);
	Soumettre(r3d, cube, NkMat4f::Translate({fenetreX, FENETRE_Y, FENETRE_Z2}) * NkMat4f::Scale({EP_PANNEAU, FENETRE_H, FENETRE_L}), fenetre);
	Soumettre(r3d, cube, NkMat4f::Translate({TABLE_X, TABLE_DESSUS - TABLE_EPAISSEUR * 0.5f, TABLE_Z}) * NkMat4f::Scale({TABLE_LONGUEUR, TABLE_EPAISSEUR, TABLE_PROFONDEUR}), bois);

	if (panneauMat) {
		const float h = 0.20f;
		const float epaisseur = 0.01f;
		const float z = faceZ + epaisseur * 0.5f + 0.003f;
		SoumettreMat(r3d, cube,
		             NkMat4f::Translate({PORTE_X, PORTE_H + 0.05f + h * 0.5f, z}) * NkMat4f::Scale({panneauLargeur, h, epaisseur}),
		             panneauMat);
	}
}

static const float PI_SALLE = 3.14159265358979f;
static const float TANGAGE_DEG = 30.f; // fixe : la souris ne pilote que le lacet

enum class Ordre { Bon, Mauvais };

static const char *NomOrdre(Ordre o) {
	return o == Ordre::Bon ? "lacet*tangage" : "tangage*lacet";
}

// L'orientation par un quaternion, dans les deux ordres de composition.
static NkQuatf Orientation(float lacetRad, Ordre ordre) {
	const NkQuatf qLacet = NkQuatf::RotateY(NkAngle(lacetRad * 180.f / PI_SALLE));
	const NkQuatf qTangage = NkQuatf::RotateX(NkAngle(TANGAGE_DEG));
	return (ordre == Ordre::Bon ? qLacet * qTangage : qTangage * qLacet).Normalized();
}

// Hauteur du regard : son angle au-dessus de l'horizon, en degres.
static float HauteurDeg(float lacetRad, Ordre ordre) {
	const NkVec3f regard = Orientation(lacetRad, ordre) * NkVec3f(0.f, 0.f, -1.f);
	float y = regard.y;
	if (y > 1.f)
		y = 1.f;
	if (y < -1.f)
		y = -1.f;
	return std::asin(y) * 180.f / PI_SALLE;
}

static void EcrireHauteurs() {
	FILE *f = std::fopen("hauteurs.txt", "w");
	const int lacets[5] = {0, 45, 90, 135, 180};
	const Ordre ordres[2] = {Ordre::Bon, Ordre::Mauvais};
	for (Ordre o : ordres) {
		for (int l : lacets) {
			const float h = HauteurDeg((float)l * PI_SALLE / 180.f, o);
			std::printf("%s, lacet %d : hauteur %.1f deg\n", NomOrdre(o), l, h);
			if (f)
				std::fprintf(f, "%s, lacet %d : hauteur %.1f deg\n", NomOrdre(o), l, h);
		}
	}
	if (f)
		std::fclose(f);
}

int nkmain(const NkEntryState &state) {
	(void)state;

	// Les dix hauteurs : au demarrage, avant la fenetre.
	EcrireHauteurs();

	// MASALLE_CALCUL_SEUL=1 : on s'arrete apres le calcul, sans ouvrir la fenetre (pratique pour mesurer).
	{
		const char *seul = std::getenv("MASALLE_CALCUL_SEUL");
		if (seul && seul[0] == '1')
			return 0;
	}
	NkWindowConfig winCfg;
	winCfg.title = "Cone";
	winCfg.width = 1280;
	winCfg.height = 720;
	winCfg.centered = true;
	winCfg.resizable = true;

	NkWindow window(winCfg);
	if (!window.IsValid()) {
		logger.Error("[Cone] Creation fenetre KO");
		return 1;
	}
	NkDeviceInitInfo devInfo{};
	devInfo.surface = window.GetSurfaceDesc();
	devInfo.width = (uint32)window.GetSize().width;
	devInfo.height = (uint32)window.GetSize().height;

	NkIDevice *device = NkDeviceFactory::CreateAutoDetect(devInfo);
	if (!device || !device->IsValid()) {
		logger.Error("[Cone] Creation device KO");
		window.Close();
		return 2;
	}
	logger.Info("[Cone] Backend : {0}", NkGraphicsApiName(device->GetApi()));

	NkRendererConfig cfg = NkRendererConfig::ForGame(devInfo.api, devInfo.width, devInfo.height);
	cfg.subsystems = NK_SS_RENDER3D;
	NkRenderer *renderer = NkRenderer::Create(device, cfg);
	if (!renderer || !renderer->Initialize()) {
		logger.Error("[Cone] Init renderer KO");
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

	// M : change d'ordre. MASALLE_ORDRE=mauvais : demarre dans l'ordre fautif.
	// MASALLE_LACET=90 : fige le lacet a 90 degres et coupe la souris (pour les captures).
	Ordre ordre = Ordre::Bon;
	if (const char *o = std::getenv("MASALLE_ORDRE"))
		if (!std::strcmp(o, "mauvais"))
			ordre = Ordre::Mauvais;
	bool lacetFige = false;
	float lacetFigeRad = 0.f;
	if (const char *l = std::getenv("MASALLE_LACET")) {
		lacetFige = true;
		lacetFigeRad = (float)std::atof(l) * PI_SALLE / 180.f;
	}
	events.AddEventCallback<NkKeyPressEvent>([&](NkKeyPressEvent *e) {
		if (e->GetKey() == NkKey::NK_M) {
			ordre = (ordre == Ordre::Bon) ? Ordre::Mauvais : Ordre::Bon;
			std::printf("ordre : %s\n", NomOrdre(ordre));
		}
	});
	int accDX = 0; // accumuler, lire une fois par image, vider
	events.AddEventCallback<NkMouseRawEvent>([&](NkMouseRawEvent *e) { accDX += e->GetDeltaX(); });

	std::printf("ordre : %s\n", NomOrdre(ordre));
	float lacet = 0.f; // en radians, non borne
	const float SENSIBILITE = 0.002f;
	while (running && window.IsOpen()) {
		events.PollEvents();
		if (!running)
			break;
		if (!renderer->BeginFrame())
			continue;

		// Position, point visé et haut de la caméra : l'exercice les calcule ci-dessous.
		NkVec3f pos = {1.5f, 1.5f, 1.7f}, cible = {-0.3f, 0.8f, -0.3f}, haut = {0.f, 1.f, 0.f};

		if (lacetFige) {
			lacet = lacetFigeRad;
			accDX = 0;
		} else {
			lacet -= (float)accDX * SENSIBILITE;
			accDX = 0;
		}
		const NkQuatf q = Orientation(lacet, ordre);
		NkVec3f tetePos = {0.30f, 1.70f, 1.20f};
		if (const char *t = std::getenv("MASALLE_TETE")) { // position de la tete, en metres : "x,y,z"
			float tx = 0.f, ty = 0.f, tz = 0.f;
			if (std::sscanf(t, "%f,%f,%f", &tx, &ty, &tz) == 3)
				tetePos = {tx, ty, tz};
		}
		pos = tetePos;
		cible = tetePos + q * NkVec3f(0.f, 0.f, -1.f); // le regard vient du quaternion
		haut = q * NkVec3f(0.f, 1.f, 0.f);               // et le haut aussi
		NkCamera3DData camData;
		camData.up = haut;
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
		ConstruireSalle(r3d, cube, nullptr, nullptr, 0.f);

		renderer->Present();
		renderer->EndFrame();
	}
	device->WaitIdle();
	NkRenderer::Destroy(renderer);
	NkDeviceFactory::Destroy(device);
	window.Close();
	return 0;
}