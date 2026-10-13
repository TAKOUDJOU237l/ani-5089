// cubes.cpp : cent cubes soumis un par un, puis groupés ; seuil de cadence.
#include <chrono>
#include <cstdio>
#include <vector>

#include "NKPlatform/NkPlatformDetect.h"
#include "NKWindow/NKMain.h"
#include "NKWindow/Core/NkWindow.h"
#include "NKWindow/Core/NkWindowConfig.h"
#include "NKEvent/NkWindowEvent.h"
#include "NKLogger/NkLog.h"

#include "NKRHI/Core/NkDeviceFactory.h"
#include "NKRenderer/NkRenderer.h"
#include "NKRenderer/Core/NkCamera.h"
#include "NKRenderer/Mesh/NkMeshSystem.h"
#include "NKRenderer/Tools/Render3D/NkRender3D.h"

using namespace nkentseu;
using namespace nkentseu::renderer;

static void ConfigureAppData(NkAppData &d) {
	d.appName = "Cubes";
}
NK_REGISTER_ENTRY_APPDATA_UPDATER(ConfigureAppData)

// ------------------------------------------------------------------ Salle
static const float INTERIEUR = 4.0f;
static const float HAUTEUR = 2.50f;
static const float EP = 0.10f;

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

// ------------------------------------------------------- Grille de cubes
static const float TAILLE_CUBE = 0.08f;
static const float PAS = 0.10f;
static const int PAR_COTE = 36;
static const int PAR_COUCHE = PAR_COTE * PAR_COTE;
static const float GRILLE_MIN = -1.8f;  // coin de départ en x et z
static const NkVec3f COULEUR_CUBE{0.9f, 0.4f, 0.2f};

static NkMat4f MatriceCube(int i) {
	const int couche = i / PAR_COUCHE;
	const int r = i % PAR_COUCHE;
	const float x = GRILLE_MIN + (r % PAR_COTE) * PAS;
	const float z = GRILLE_MIN + (r / PAR_COTE) * PAS;
	const float y = 0.10f + couche * PAS;
	return NkMat4f::Translate({x, y, z}) * NkMat4f::Scale({TAILLE_CUBE, TAILLE_CUBE, TAILLE_CUBE});
}

// Boîte englobante du groupe, en coordonnées du monde : elle doit contenir
// TOUS les cubes, sinon le renderer écarte le groupe en silence.
static NkAABB BoiteDuGroupe(int n) {
	const int couches = (n + PAR_COUCHE - 1) / PAR_COUCHE;
	const float marge = TAILLE_CUBE * 0.5f;
	const float lo = GRILLE_MIN - marge;
	const float hi = GRILLE_MIN + (PAR_COTE - 1) * PAS + marge;
	const float yLo = 0.10f - marge;
	const float yHi = 0.10f + (couches - 1) * PAS + marge;
	const NkVec3f centre{(lo + hi) * 0.5f, (yLo + yHi) * 0.5f, (lo + hi) * 0.5f};
	const NkVec3f taille{hi - lo, yHi - yLo, hi - lo};
	return NkAABB::Unit().Transformed(NkMat4f::Translate(centre) * NkMat4f::Scale(taille));
}

// ---------------------------------------------------------------- Mesure
struct Resultat {
	double fps;
	int drawCalls;
};

// Mesure la cadence sur 'images' images, avec n cubes soumis un par un
// (groupe = false) ou d'un seul appel via NkDrawCallInstanced (groupe = true).
static Resultat Mesurer(NkRenderer *renderer, NkRender3D *r3d, NkMeshHandle cube, NkMeshHandle plan,
                        int n, bool groupe, int images, bool afficher, uint32 W, uint32 H) {
	NkEventSystem &events = NkEvents();
	const int CHAUFFE = 10;  // premières images : chargements, non comptées

	// Préparation hors boucle : même travail CPU pour les deux façons.
	std::vector<NkMat4f> matrices;
	matrices.reserve(n);
	for (int i = 0; i < n; ++i)
		matrices.push_back(MatriceCube(i));

	const int PAQUET = 128;
	std::vector<NkDrawCallInstanced> paquets;
	if (groupe && n > 0) {
		const NkAABB boite = BoiteDuGroupe(n);
		for (int debut = 0; debut < n; debut += PAQUET) {
			NkDrawCallInstanced dc;
			dc.mesh = cube;
			dc.aabb = boite;
			const int fin = (debut + PAQUET < n) ? debut + PAQUET : n;
			for (int i = debut; i < fin; ++i) {
				dc.transforms.PushBack(matrices[i]);
				dc.tints.PushBack(COULEUR_CUBE);
			}
			paquets.push_back(dc);
		}
	}

	int frame = 0;
	int drawCalls = 0;
	auto t0 = std::chrono::steady_clock::now();
	auto tPalier = t0;

	while (frame < CHAUFFE + images) {
		events.PollEvents();
		if (!renderer->BeginFrame())
			continue;

		NkCamera3DData cd;
		cd.up = {0.f, 1.f, 0.f};
		cd.fovY = 70.f;
		cd.aspect = (float32)W / (float32)H;
		cd.nearPlane = 0.05f;
		cd.farPlane = 200.f;
		NkCamera3D cam(cd);
		cam.SetPosition({1.8f, 1.70f, 1.8f});
		cam.SetTarget({-0.5f, 1.2f, -0.5f});

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
		ConstruireSalle(r3d, cube, plan);
		if (n > 0) {
			if (groupe) {
				for (size_t p = 0; p < paquets.size(); ++p)
					r3d->SubmitInstanced(paquets[p]);
			} else {
				for (int i = 0; i < n; ++i)
					Soumettre(r3d, cube, matrices[i], COULEUR_CUBE);
			}
		}
		renderer->Present();
		renderer->EndFrame();

		drawCalls = (int)renderer->GetStats().drawCalls;
		++frame;
		if (frame == CHAUFFE) {
			t0 = std::chrono::steady_clock::now();
			tPalier = t0;
		}
		// Affichage console : cadence et drawCalls tous les 100 images.
		if (afficher && frame > CHAUFFE && (frame - CHAUFFE) % 100 == 0) {
			auto now = std::chrono::steady_clock::now();
			std::chrono::duration<double> dt = now - tPalier;
			std::printf("    image %d/%d : %.1f fps, drawCalls %d\n", frame - CHAUFFE, images, 100.0 / dt.count(), drawCalls);
			std::fflush(stdout);
			tPalier = now;
		}
	}

	std::chrono::duration<double> dt = std::chrono::steady_clock::now() - t0;
	Resultat r;
	r.fps = images / dt.count();
	r.drawCalls = drawCalls;
	return r;
}

// Cherche le nombre de cubes à partir duquel la cadence tombe sous 'cible'.
static int ChercherSeuil(NkRenderer *rd, NkRender3D *r3d, NkMeshHandle cube, NkMeshHandle plan,
                         bool groupe, double cible, int depart, uint32 W, uint32 H) {
	int n = depart;
	for (;;) {
		Resultat r = Mesurer(rd, r3d, cube, plan, n, groupe, 60, false, W, H);
		std::printf("  essai %s : %d cubes -> %.1f fps, drawCalls %d\n",
		            groupe ? "groupes" : "un par un", n, r.fps, r.drawCalls);
		std::fflush(stdout);
		if (r.fps < cible || n > 300000)
			return n;
		n = (int)(n * 1.5) + 50;
	}
}

// ------------------------------------------------------------------ Main
int nkmain(const NkEntryState &state) {
	(void)state;

	NkWindowConfig winCfg;
	winCfg.title = "Cubes";
	winCfg.width = 1280;
	winCfg.height = 720;
	winCfg.centered = true;

	NkWindow window(winCfg);
	if (!window.IsValid()) {
		logger.Error("[Cubes] Creation fenetre KO");
		return 1;
	}

	NkDeviceInitInfo devInfo{};
	devInfo.surface = window.GetSurfaceDesc();
	devInfo.width = (uint32)window.GetSize().width;
	devInfo.height = (uint32)window.GetSize().height;

	NkIDevice *device = NkDeviceFactory::CreateForApi(NkGraphicsApi::NK_GFX_API_OPENGL, devInfo);
	if (!device || !device->IsValid()) {
		logger.Error("[Cubes] Creation device KO");
		window.Close();
		return 2;
	}
	logger.Info("[Cubes] Backend : {0}", NkGraphicsApiName(device->GetApi()));

	// La croix est ignorée pendant la mesure : fermer la fenêtre stopperait tout.
	NkEventSystem &events = NkEvents();
	events.AddEventCallback<NkWindowCloseEvent>([](NkWindowCloseEvent *) {
		std::printf("Mesure en cours : ne ferme pas la fenetre.\n");
		std::fflush(stdout);
	});

	NkRendererConfig cfg = NkRendererConfig::ForGame(device->GetApi(), devInfo.width, devInfo.height);
	cfg.subsystems = NK_SS_RENDER3D;
	cfg.vsync = false;  // la cadence n'est plus plafonnée par l'écran
	NkRenderer *renderer = NkRenderer::Create(device, cfg);
	if (!renderer || !renderer->Initialize()) {
		logger.Error("[Cubes] Init renderer KO");
		NkRenderer::Destroy(renderer);
		NkDeviceFactory::Destroy(device);
		window.Close();
		return 3;
	}
	NkRender3D *r3d = renderer->GetRender3D();
	NkMeshHandle cube = renderer->GetMeshSystem()->GetCube();
	NkMeshHandle plan = renderer->GetMeshSystem()->GetPlane();
	const uint32 W = devInfo.width;
	const uint32 H = devInfo.height;

	// 1. Salle seule : si elle est déjà sous 60 fps, le seuil vaut la moitié de sa cadence.
	std::printf("Salle seule :\n");
	Resultat base = Mesurer(renderer, r3d, cube, plan, 0, false, 400, true, W, H);
	const double cible = (base.fps >= 60.0) ? 60.0 : base.fps / 2.0;
	std::printf("Salle seule : %.1f fps, drawCalls %d -> seuil = %.1f fps\n", base.fps, base.drawCalls, cible);

	// 2. Un par un : seuil, puis mesure sur 400 images.
	std::printf("Un par un : recherche du seuil\n");
	const int n1 = ChercherSeuil(renderer, r3d, cube, plan, false, cible, 50, W, H);
	std::printf("Un par un : mesure de %d cubes\n", n1);
	Resultat a = Mesurer(renderer, r3d, cube, plan, n1, false, 400, true, W, H);
	std::printf("Un par un : %d cubes, drawCalls %d, %.1f fps\n", n1, a.drawCalls, a.fps);

	// 3. Groupés, mêmes cubes.
	std::printf("Groupes : mesure de %d cubes\n", n1);
	Resultat b = Mesurer(renderer, r3d, cube, plan, n1, true, 400, true, W, H);
	std::printf("Groupes : %d cubes, drawCalls %d, %.1f fps\n", n1, b.drawCalls, b.fps);

	// 4. Groupés, nouveau seuil.
	std::printf("Groupes : recherche du nouveau seuil\n");
	const int n2 = ChercherSeuil(renderer, r3d, cube, plan, true, cible, n1, W, H);
	std::printf("Groupes : nouveau seuil = %d cubes\n", n2);

	FILE *f = std::fopen("cadence_mesures.txt", "w");
	if (f) {
		std::fprintf(f, "Salle seule : %.1f fps, drawCalls %d, seuil %.1f fps\n", base.fps, base.drawCalls, cible);
		std::fprintf(f, "Un par un : %d cubes, drawCalls %d, %.1f fps\n", n1, a.drawCalls, a.fps);
		std::fprintf(f, "Groupes : drawCalls %d, %.1f fps, nouveau seuil %d cubes\n", b.drawCalls, b.fps, n2);
		std::fclose(f);
	}

	device->WaitIdle();
	NkRenderer::Destroy(renderer);
	NkDeviceFactory::Destroy(device);
	window.Close();
	std::printf("Termine : voir cadence_mesures.txt\n");
	return 0;
}