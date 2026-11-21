// =============================================================================
//  Exercice 10 (chapitre 10) : la boucle qui ne reprend jamais.
//                                                      *** VERSION SIMULEE ***
//
//  Pas de vrai casque : la pause est scriptee (shouldRender vaut faux aux
//  trames 8 a 12). La session est simulee : elle refuse une attente alors
//  qu'une trame est ouverte (sur un vrai casque, l'attente se bloquerait).
//
//    BOUCLE_FAUTIVE=1 : la fin de trame est DANS le test de rendu -> journal.txt, avant.png
//    (rien)           : la fin de trame est HORS du test           -> journal-apres.txt, apres.png
//  Journal : une ligne par trame, 20 trames. Capture a la trame 16.
// =============================================================================
#include "NKWindow/NKMain.h"
#include "NKWindow/Core/NkWindow.h"
#include "NKWindow/Core/NkWindowConfig.h"
#include "NKEvent/NkEventSystem.h"
#include "NKEvent/NkWindowEvent.h"

#include "NKRHI/Core/NkDeviceFactory.h"
#include "NKRenderer/NkRenderer.h"
#include "NKRenderer/Core/NkCamera.h"
#include "NKRenderer/Core/NkRenderGraph.h"
#include "NKRenderer/Core/NkTextureLibrary.h"
#include "NKRenderer/Materials/NkMaterialCollection.h"
#include "NKRenderer/Mesh/NkMeshSystem.h"
#include "NKRenderer/Tools/Offscreen/NkOffscreenTarget.h"
#include "NKRenderer/Tools/Render2D/NkRender2D.h"
#include "NKRenderer/Tools/Render3D/NkRender3D.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>

using namespace nkentseu;
using namespace nkentseu::renderer;

static void ConfigureAppData(NkAppData &d) { d.appName = "SimCouches"; }
NK_REGISTER_ENTRY_APPDATA_UPDATER(ConfigureAppData)

namespace {

	// ── Les dimensions de la salle, en un seul endroit ────────────────────
	// Elles sont ici, nommées, et pas semées dans le code : au chapitre 21 on
	// les lira depuis un fichier, et ce sont ces noms-là qui deviendront des
	// champs.
	const float32 kPiece = 4.00f;	  // côté intérieur du sol
	const float32 kPlafond = 2.50f;	  // hauteur sous plafond
	const float32 kMur = 0.10f;		  // épaisseur d'un mur
	const float32 kPorteL = 0.90f;	  // porte : largeur
	const float32 kPorteH = 2.00f;	  //         hauteur
	const float32 kFenL = 1.20f;	  // fenêtre : largeur
	const float32 kFenH = 1.00f;	  //           hauteur
	const float32 kFenY = 1.40f;	  //           hauteur de son centre
	const float32 kTableL = 1.20f;	  // table : longueur
	const float32 kTableP = 0.80f;	  //         profondeur
	const float32 kTableH = 0.75f;	  //         hauteur du plateau
	const float32 kYeux = 1.70f;	  // hauteur des yeux, debout

	const float32 kDemi = kPiece * 0.5f;

	// Un pavé posé dans la salle. `centre` est le centre géométrique, `taille`
	// les trois dimensions en mètres : on ne manipule jamais autre chose.
	void Pave(NkRender3D *r3d, NkMeshSystem *meshes, const NkVec3f &centre,
			  const NkVec3f &taille, const NkVec3f &teinte, float32 rugosite = 1.f) {
		NkDrawCall3D dc;
		dc.mesh = meshes->GetCube();
		dc.transform = NkMat4f::Translate(centre) * NkMat4f::Scale(taille);
		dc.aabb = { { centre.x - taille.x * 0.5f, centre.y - taille.y * 0.5f,
					  centre.z - taille.z * 0.5f },
					{ centre.x + taille.x * 0.5f, centre.y + taille.y * 0.5f,
					  centre.z + taille.z * 0.5f } };
		dc.tint = teinte;
		dc.metallic = 0.f;
		dc.roughness = rugosite;
		r3d->Submit(dc);
	}

	void SoumettreLaSalle(NkRender3D *r3d, NkMeshSystem *meshes) {
		const NkVec3f grisMur = { 0.72f, 0.70f, 0.66f };
		const NkVec3f grisSol = { 0.38f, 0.36f, 0.34f };
		const NkVec3f bois = { 0.55f, 0.38f, 0.22f };
		const NkVec3f vitre = { 0.75f, 0.85f, 0.92f };

		{ // le sol : le plan est unitaire, une échelle S donne un côté S
			NkDrawCall3D dc;
			dc.mesh = meshes->GetPlane();
			dc.transform = NkMat4f::Scale({ kPiece, 1.f, kPiece });
			dc.aabb = { { -kDemi, 0.f, -kDemi }, { kDemi, 0.f, kDemi } };
			dc.castShadow = false;
			dc.tint = grisSol;
			dc.metallic = 0.f;
			dc.roughness = 1.f;
			r3d->Submit(dc);
		}

		const float32 yMur = kPlafond * 0.5f;
		const float32 bord = kDemi + kMur * 0.5f;

		// Quatre murs. La porte est dans celui du fond, les fenêtres à gauche.
		Pave(r3d, meshes, { 0.f, yMur, -bord }, { kPiece, kPlafond, kMur }, grisMur);
		Pave(r3d, meshes, { 0.f, yMur, bord }, { kPiece, kPlafond, kMur }, grisMur);
		Pave(r3d, meshes, { -bord, yMur, 0.f }, { kMur, kPlafond, kPiece }, grisMur);
		Pave(r3d, meshes, { bord, yMur, 0.f }, { kMur, kPlafond, kPiece }, grisMur);

		// Le plafond. On l'oublie toujours, et le trou ne se voit qu'en levant
		// les yeux, c'est-à-dire le jour de la démonstration.
		Pave(r3d, meshes, { 0.f, kPlafond + kMur * 0.5f, 0.f },
			 { kPiece, kMur, kPiece }, grisMur);

		// La porte, fermée jusqu'au chapitre 17 : un panneau plaqué contre le
		// mur du fond, décalé pour ne pas se battre avec lui en profondeur.
		Pave(r3d, meshes, { -0.70f, kPorteH * 0.5f, -kDemi + 0.02f },
			 { kPorteL, kPorteH, 0.04f }, bois, 0.6f);

		// Deux fenêtres sur le mur de gauche, pour qu'il y ait un dehors.
		Pave(r3d, meshes, { -kDemi + 0.02f, kFenY, -0.90f },
			 { 0.04f, kFenH, kFenL }, vitre, 0.15f);
		Pave(r3d, meshes, { -kDemi + 0.02f, kFenY, 0.90f },
			 { 0.04f, kFenH, kFenL }, vitre, 0.15f);

		// La table : un plateau et quatre pieds. Le chapitre 12 y posera un
		// objet, le 21 la déplacera.
		const float32 tx = 0.60f, tz = 0.70f;
		const float32 ep = 0.05f;
		Pave(r3d, meshes, { tx, kTableH - ep * 0.5f, tz },
			 { kTableL, ep, kTableP }, bois, 0.5f);

		const float32 pied = 0.06f;
		const float32 hp = kTableH - ep;
		for (int i = 0; i < 4; ++i) {
			const float32 dx = ((i & 1) ? 1.f : -1.f) * (kTableL * 0.5f - pied);
			const float32 dz = ((i & 2) ? 1.f : -1.f) * (kTableP * 0.5f - pied);
			Pave(r3d, meshes, { tx + dx, hp * 0.5f, tz + dz },
				 { pied, hp, pied }, bois, 0.6f);
		}

		// Le repère d'un mètre du chapitre 9 : un cube d'un mètre de côté,
		// posé au sol, contre lequel on juge l'échelle de tout le reste.
		Pave(r3d, meshes, { -1.10f, 0.50f, 1.20f }, { 1.f, 1.f, 1.f },
			 { 0.85f, 0.80f, 0.30f }, 0.8f);
	}

	uint64 EnvU64(const char *nom, uint64 defaut) {
		const char *t = getenv(nom);
		if (t == nullptr || *t == '\0') return defaut;
		return (uint64)strtoull(t, nullptr, 10);
	}

} // namespace

// ── Logique simulee ──
namespace {
	// Dessine UNE image de la salle dans la cible, avec la camera donnee.
	void DessinerImage(NkRenderer *renderer, NkRenderer *rEcran, NkICommandBuffer *cmd,
					   const NkVec3f &pos, const NkVec3f &cible3, float32 aspect,
					   float32 temps, void (*contenu)(NkRender3D *, NkMeshSystem *, const void *), const void *donnees) {
		NkRender3D *r3d = renderer->GetRender3D();
		renderer->FlushGraphRebuilds();
		r3d->ResetFrame();
		if (auto *mc = renderer->GetMaterialCollection()) mc->Upload();

		NkCamera3DData cd;
		cd.position = pos;
		cd.target = cible3;
		cd.up = { 0.f, 1.f, 0.f };
		cd.fovY = 70.f;
		cd.aspect = aspect;
		cd.nearPlane = 0.05f;
		cd.farPlane = 100.f;

		NkSceneContext sctx;
		sctx.camera = NkCamera3D(cd);
		sctx.time = temps;
		r3d->SetIBLStrength(0.35f);
		NkLightDesc soleil;
		soleil.type = NkLightType::NK_DIRECTIONAL;
		soleil.direction = { -0.5f, -1.f, -0.35f };
		soleil.color = { 1.f, 0.96f, 0.88f };
		soleil.intensity = 3.f;
		soleil.castShadow = false;
		sctx.lights.PushBack(soleil);

		r3d->BeginScene(sctx);
		SoumettreLaSalle(r3d, renderer->GetMeshSystem());
		contenu(r3d, renderer->GetMeshSystem(), donnees);
		if (auto *graph = renderer->GetRenderGraph()) graph->Execute(cmd);
		(void)rEcran;
	}
} // namespace

// ── Logique simulee : la session, et la boucle ─────────────────────────────
// Le simulateur du chapitre VALIDE au lieu de composer. Ici : il compte les
// trames, et refuse une attente alors qu'une trame est encore ouverte. Sur un
// vrai casque cette attente ne reviendrait jamais ; on la fait ECHOUER pour que
// le programme puisse continuer et que le journal montre ce qui se passe.
namespace {
struct SessionSim {
	bool ouverte = false;     // une trame a ete commencee et pas terminee
	int  refus = 0;
	bool Attendre() { if (ouverte) { ++refus; return false; } return true; }
	void Commencer() { ouverte = true; }
	void Finir() { ouverte = false; }
};

struct LigneJournal { int trame = 0; int rend = 0; int debut = 0; int fin = 0; bool refuse = false; };

struct Boucle {
	SessionSim session;
	bool fautive = false;     // true : la fin de trame est DANS le test de rendu
	int  imageAffichee = 0;   // la derniere trame dont l'image a ete dessinee

	// Le casque est pose aux trames 8 a 12 : shouldRender vaut faux.
	static bool DoitDessiner(int trame) { return !(trame >= 8 && trame <= 12); }

	// Un tour de boucle : attente, debut, actions, rendu conditionnel, fin.
	LigneJournal Tour(int trame) {
		LigneJournal l;
		l.trame = trame;
		// 1. (la pompe d'evenements est dans la boucle du moteur)
		// 2. attente de la trame
		if (!session.Attendre()) { l.refuse = true; return l; }
		// 3. debut de trame
		session.Commencer(); l.debut = 1;
		// 4. synchronisation des actions : rien a faire ici
		const bool shouldRender = DoitDessiner(trame);
		if (shouldRender) {
			// 5. rendu : localisation des vues, acquisition, dessin, liberation, couche
			l.rend = 1; imageAffichee = trame;
			// VERSION FAUTIVE : la fin de trame DANS le test de rendu.
			if (fautive) { session.Finir(); l.fin = 1; }
		}
		// VERSION JUSTE : la fin de trame HORS du test, a chaque tour, meme sans rien dessiner.
		if (!fautive) { session.Finir(); l.fin = 1; }
		return l;
	}
};

// La scene : un cube qui avance d'un pas par trame, pour que le gel se voie.
struct Cube { double x; };
void DessinerCube(NkRender3D *r3d, NkMeshSystem *meshes, const void *donnees) {
	const Cube *c = static_cast<const Cube *>(donnees);
	Pave(r3d, meshes, { (float32)c->x, 1.10f, 0.20f }, { 0.18f, 0.18f, 0.18f }, { 0.15f, 0.75f, 0.30f });
}
} // namespace

int nkmain(const NkEntryState &state) {
	(void)state;
	NkWindowConfig cfg;
	cfg.title = "Simulation couches";
	cfg.width = 1280;
	cfg.height = 720;
	cfg.centered = true;
	cfg.vsync = true;

	NkWindow fenetre(cfg);
	if (!fenetre.IsValid()) { std::printf("[Sim] la fenetre n'a pas pu s'ouvrir\n"); return 1; }

	NkDeviceInitInfo devInit{};
	devInit.surface = fenetre.GetSurfaceDesc();
	devInit.width = fenetre.GetSize().x;
	devInit.height = fenetre.GetSize().y;
	NkIDevice *device = NkDeviceFactory::CreateAutoDetect(devInit);
	if (device == nullptr) { std::printf("[Sim] aucune API graphique\n"); return 1; }

	NkRendererConfig cfgEcran = NkRendererConfig::For2D(devInit.api, devInit.width, devInit.height);
	NkRenderer *rEcran = NkRenderer::Create(device, cfgEcran);
	if (rEcran == nullptr || !rEcran->Initialize()) { std::printf("[Sim] renderer ecran KO\n"); return 1; }

	const uint32 L = 640, H = 360;
	NkOffscreenDesc od;
	od.width = L; od.height = H; od.hasDepth = true; od.hdr = false;
	od.colorFmt = NkGPUFormat::NK_RGBA8_UNORM;
	od.readable = true; od.readback = true; od.name = "Sim_Cible";
	NkOffscreenTarget *cible = rEcran->CreateOffscreen(od);
	if (cible == nullptr) { std::printf("[Sim] cible refusee\n"); return 1; }

	NkRendererConfig cfgSalle = NkRendererConfig::ForGame(devInit.api, devInit.width, devInit.height);
	cfgSalle.Disable(NK_SS_RENDER2D);
	cfgSalle.Disable(NK_SS_TEXT);
	cfgSalle.Disable(NK_SS_OVERLAY);
	cfgSalle.Enable(NK_SS_OFFSCREEN);
	NkRenderer *renderer = NkRenderer::Create(device, cfgSalle);
	if (renderer == nullptr || !renderer->Initialize()) { std::printf("[Sim] renderer salle KO\n"); return 1; }
	renderer->SetFinalColorTarget(rEcran->GetTextures()->GetRHIHandle(cible->GetColorResult()));
	renderer->SetRenderSizeOverride(L, H);

	// BOUCLE_FAUTIVE=1 : la fin de trame DANS le test de rendu. Rien : version corrigee.
	const bool fautive = EnvU64("BOUCLE_FAUTIVE", 0) != 0;
	Boucle boucle;
	boucle.fautive = fautive;
	std::printf("[Sim] SIMULATION : pas de vrai casque, la pause est scriptee (trames 8 a 12). Mode : %s\n",
				fautive ? "FAUTIVE (fin dans le test)" : "CORRIGEE (fin hors du test)");
	std::FILE *journal = std::fopen(fautive ? "journal.txt" : "journal-apres.txt", "w");

	const uint64 imageDeCapture = 16;   // apres la pause : c'est la qu'on voit si la boucle a repris
	const uint64 imageDeSortie = 20;
	uint64 image = 0;
	bool tourne = true;
	while (tourne && fenetre.IsOpen()) {
		NkEvent *ev = nullptr;                 // pompe d'evenements
		while (NkEvents().PollEvent(ev)) {
			if (ev->As<NkWindowCloseEvent>() != nullptr) tourne = false;
		}
		if (!tourne) break;

		const int trame = (int)image + 1;
		const LigneJournal l = boucle.Tour(trame);
		if (l.refuse) {
			std::printf("[Sim] trame %d : ATTEND alors qu'une trame est ouverte. Sur un vrai casque la boucle se bloquerait ici.\n", trame);
		}

		if (rEcran->BeginFrame()) {
			NkICommandBuffer *cmd = rEcran->GetCmd();
			if (l.rend) {                      // on ne redessine que si la trame a rendu
				const Cube cube = { -0.60 + 0.08 * (double)boucle.imageAffichee };
				DessinerImage(renderer, rEcran, cmd, { 0.30f, 1.50f, 1.90f }, { 0.20f, 0.90f, 0.50f },
							  (float32)L / (float32)H, (float32)trame / 60.f, DessinerCube, &cube);
			}
			// sinon la cible garde sa derniere image : c'est l'ecran fige.
			if (auto *r2d = rEcran->GetRender2D()) {
				r2d->DrawImage(cible->GetColorResult(), { 0.f, 0.f, (float32)devInit.width, (float32)devInit.height });
			}
			rEcran->Present();
			rEcran->EndFrame();
		}
		++image;

		if (journal != nullptr) {
			std::fprintf(journal, "trame : %d, rend : %d, debut : %d, fin : %d\n", l.trame, l.rend, l.debut, l.fin);
			std::fflush(journal);
		}
		if (image == imageDeCapture) {
			const char *fichier = fautive ? "avant.png" : "apres.png";
			const bool ok = cible->Capture(fichier);
			std::printf("[Sim] trame %llu : derniere image dessinee = trame %d -> %s (%s)\n",
						(unsigned long long)image, boucle.imageAffichee, fichier, ok ? "ok" : "echec");
		}
		if (image >= imageDeSortie) tourne = false;
	}
	std::printf("[Sim] %llu trames, attentes refusees : %d\n", (unsigned long long)image, boucle.session.refus);
	if (journal != nullptr) std::fclose(journal);

	NkRenderer::Destroy(renderer);
	NkRenderer::Destroy(rEcran);
	NkDeviceFactory::Destroy(device);
	return 0;
}
