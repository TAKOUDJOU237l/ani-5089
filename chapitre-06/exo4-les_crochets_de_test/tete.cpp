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

using namespace nkentseu;
using namespace nkentseu::renderer;

static void ConfigureAppData(NkAppData &d) {
	d.appName = "Tete";
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

// Pour l'essai de l'exercice (signe du lacet inverse dans Regard) : MASALLE_SIGNE_INVERSE=1.
// Ce reglage ne sert qu'a provoquer l'echec voulu du test ; il n'est pas actif par defaut.
static float SigneDuLacet() {
	const char *v = std::getenv("MASALLE_SIGNE_INVERSE");
	return (v && v[0] == '1') ? -1.f : 1.f;
}

// La tete : le lacet et le tangage en radians, la position en metres.
struct Tete {
	float lacet = 0.f;
	float tangage = 0.f;
	NkVec3f position = {0.f, 0.f, 0.f};

	// Direction du regard : (-sin(lacet).cos(tangage) ; sin(tangage) ; -cos(lacet).cos(tangage)).
	NkVec3f Regard() const {
		const float l = SigneDuLacet() * lacet;
		return {-std::sin(l) * std::cos(tangage), std::sin(tangage), -std::cos(lacet) * std::cos(tangage)};
	}

	// Pose la tete directement, sans passer par la souris : c'est le crochet de test.
	void PoserTete(float lacetRad, float tangageRad, const NkVec3f &p) {
		lacet = lacetRad;
		tangage = tangageRad;
		position = p;
	}
};

// -----------------------------------------------------------------------------------------
// Le test : il tourne sans fenetre, quand MASALLE_TEST vaut 1, et rend son verdict par le code de sortie de nkmain.
// Les directions attendues sont ECRITES EN DUR, deduites de la convention de la salle
// (avant = -Z, haut = +Y, repere de la main droite) ; elles ne sont jamais recalculees avec Regard().
//
//   lacet 0   : on regarde l'avant de la salle, -Z                          -> (0, 0, -1)
//   lacet 90  : tourner de +90 degres autour de Y (+Y vers le haut, main droite) amene -Z sur -X
//               (le regard va vers la gauche, comme un cou qui tourne a gauche) -> (-1, 0, 0)
//   lacet 180 : demi-tour, +Z                                                 -> (0, 0, 1)
//   tangage 30 : lever les yeux de 30 degres : y = sin 30 = 0,5, z = -cos 30  -> (0, 0.5, -0.8660)
//   lacet 90 et tangage 45 : -X incline de 45 degres vers le haut             -> (-0.7071, 0.7071, 0)
// -----------------------------------------------------------------------------------------
struct Epreuve {
	float lacetDeg, tangageDeg;
	float attendu[3];
};

static float Net(float v) {
	return std::fabs(v) < 5e-5f ? 0.f : v; // evite d'afficher -0.0000
}

static int LancerTests() {
	const Epreuve epreuves[] = {
	    {90.f, 0.f, {-1.f, 0.f, 0.f}},
	    {0.f, 0.f, {0.f, 0.f, -1.f}},
	    {180.f, 0.f, {0.f, 0.f, 1.f}},
	    {0.f, 30.f, {0.f, 0.5f, -0.8660254f}},
	    {90.f, 45.f, {-0.70710678f, 0.70710678f, 0.f}},
	};
	const float TOLERANCE = 1e-4f; // cos(90 deg) en float ne vaut pas exactement zero
	int echecs = 0;
	for (const Epreuve &e : epreuves) {
		Tete tete;
		tete.PoserTete(e.lacetDeg * PI_SALLE / 180.f, e.tangageDeg * PI_SALLE / 180.f, {0.f, 1.7f, 0.f});
		const NkVec3f r = tete.Regard();
		const bool ok = std::fabs(r.x - e.attendu[0]) < TOLERANCE && std::fabs(r.y - e.attendu[1]) < TOLERANCE &&
		                std::fabs(r.z - e.attendu[2]) < TOLERANCE;
		if (!ok)
			++echecs;
		std::printf("lacet %g, tangage %g : attendu (%.4f, %.4f, %.4f), obtenu (%.4f, %.4f, %.4f), %s\n", e.lacetDeg, e.tangageDeg,
		            Net(e.attendu[0]), Net(e.attendu[1]), Net(e.attendu[2]), Net(r.x), Net(r.y), Net(r.z), ok ? "OK" : "ECHEC");
	}
	return echecs == 0 ? 0 : 1;
}

int nkmain(const NkEntryState &state) {
	(void)state;

	// Le test tourne AVANT l'ouverture de la fenetre, quand MASALLE_TEST vaut 1.
	{
		const char *test = std::getenv("MASALLE_TEST");
		if (test && test[0] == '1')
			return LancerTests();
	}
	NkWindowConfig winCfg;
	winCfg.title = "Tete";
	winCfg.width = 1280;
	winCfg.height = 720;
	winCfg.centered = true;
	winCfg.resizable = true;

	NkWindow window(winCfg);
	if (!window.IsValid()) {
		logger.Error("[Tete] Creation fenetre KO");
		return 1;
	}
	NkDeviceInitInfo devInfo{};
	devInfo.surface = window.GetSurfaceDesc();
	devInfo.width = (uint32)window.GetSize().width;
	devInfo.height = (uint32)window.GetSize().height;

	NkIDevice *device = NkDeviceFactory::CreateAutoDetect(devInfo);
	if (!device || !device->IsValid()) {
		logger.Error("[Tete] Creation device KO");
		window.Close();
		return 2;
	}
	logger.Info("[Tete] Backend : {0}", NkGraphicsApiName(device->GetApi()));

	NkRendererConfig cfg = NkRendererConfig::ForGame(devInfo.api, devInfo.width, devInfo.height);
	cfg.subsystems = NK_SS_RENDER3D;
	NkRenderer *renderer = NkRenderer::Create(device, cfg);
	if (!renderer || !renderer->Initialize()) {
		logger.Error("[Tete] Init renderer KO");
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

	int accDX = 0, accDY = 0;
	events.AddEventCallback<NkMouseRawEvent>([&](NkMouseRawEvent *e) {
		accDX += e->GetDeltaX();
		accDY += e->GetDeltaY();
	});

	Tete tete;
	tete.PoserTete(0.f, 0.f, {0.30f, 1.70f, 1.20f}); // la place de la camera de la salle
	const float SENSIBILITE = 0.002f;
	const float LIMITE = 89.f * PI_SALLE / 180.f; // le tangage est borne a 89 degres
	while (running && window.IsOpen()) {
		events.PollEvents();
		if (!running)
			break;
		if (!renderer->BeginFrame())
			continue;

		// Position, point visé et haut de la caméra : l'exercice les calcule ci-dessous.
		NkVec3f pos = {1.5f, 1.5f, 1.7f}, cible = {-0.3f, 0.8f, -0.3f}, haut = {0.f, 1.f, 0.f};

		// La souris pose la tete : on lit l'accumulation une fois par image, puis on la vide.
		tete.lacet -= (float)accDX * SENSIBILITE; // le lacet n'est pas borne
		tete.tangage -= (float)accDY * SENSIBILITE;
		accDX = 0;
		accDY = 0;
		if (tete.tangage > LIMITE)
			tete.tangage = LIMITE;
		if (tete.tangage < -LIMITE)
			tete.tangage = -LIMITE;
		pos = tete.position;
		cible = tete.position + tete.Regard();
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
