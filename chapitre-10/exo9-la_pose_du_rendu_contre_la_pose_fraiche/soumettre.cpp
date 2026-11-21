// =============================================================================
//  Exercice 9 (chapitre 10) : la pose du rendu, contre la pose fraiche.
//                                                      *** VERSION SIMULEE ***
//
//  Le moteur n'a pas de compositeur, donc pas de vraie reprojection : on la
//  calcule. La tete tourne PENDANT le rendu (a 360 degres par seconde, vitesse
//  exageree pour que l'ecart se voie : une tete reelle va 5 a 10 fois moins vite).
//  Le compositeur applique une correction de (reel - soumis) degres.
//
//    (rien)              : version JUSTE   - on soumet la pose du RENDU
//    SOUMETTRE_FRAICHE=1 : version FAUTIVE - on redemande la pose pour l'instant courant
//  Capture a l'image 30, quand la tete regarde droit devant : rendu.png (juste)
//  ou fraiche.png (fautive).
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

// ── Logique simulee : la tete, et le compositeur ───────────────────────────
// Le moteur n'a pas de compositeur : c'est la reprojection que l'on SIMULE ici.
// Le compositeur d'un vrai casque connait la pose reelle au moment d'afficher,
// et applique a l'image une correction de (reel - soumis) degres. L'image
// parait alors vue depuis  rendu + (reel - soumis),  alors que la tete est a
// `reel` : l'erreur qui reste a l'ecran vaut  rendu - soumis.
namespace {
const double kPi = 3.14159265358979323846;
const double kVitesseTete = 360.0;      // degres par seconde : EXAGEREE, pour que l'ecart se voie
const double kDureeRendu = 1.0 / 60.0;  // la tete bouge pendant le rendu : une trame

// L'angle (lacet) de la tete, en degres, a l'instant `secondes`. A 0,5 s (image 30)
// la tete regarde droit devant elle : lacet 0.
static double Lacet(double secondes) { return kVitesseTete * (secondes - 0.5); }

// L'angle sous lequel l'image corrigee parait vue.
static double Apparent(double reel, double rendu, double soumis) {
	return reel + (rendu - soumis);
}
} // namespace

namespace {
// Deux poteaux dans la salle, immobiles : c'est contre eux qu'on voit le glissement.
void DessinerPoteaux(NkRender3D *r3d, NkMeshSystem *meshes, const void *) {
	Pave(r3d, meshes, { 0.30f, 1.00f, -1.80f }, { 0.08f, 2.00f, 0.08f }, { 0.90f, 0.10f, 0.10f });
	Pave(r3d, meshes, { -0.50f, 1.00f, -1.80f }, { 0.08f, 2.00f, 0.08f }, { 0.10f, 0.30f, 0.90f });
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

	// SOUMETTRE_FRAICHE=1 : version FAUTIVE (pose fraiche). Rien : version JUSTE.
	const bool fraiche = EnvU64("SOUMETTRE_FRAICHE", 0) != 0;
	std::printf("[Sim] SIMULATION : pas de compositeur, la reprojection est calculee. Mode : %s\n",
				fraiche ? "FAUTIF (pose fraiche)" : "JUSTE (pose du rendu)");

	const uint64 imageDeCapture = 30;   // la meme image du mouvement pour les deux captures
	uint64 image = 0;
	bool tourne = true;
	while (tourne && fenetre.IsOpen()) {
		NkEvent *ev = nullptr;
		while (NkEvents().PollEvent(ev)) {
			if (ev->As<NkWindowCloseEvent>() != nullptr) tourne = false;
		}
		if (!tourne) break;

		const double temps = (double)(image + 1) / 60.0;

		// 1. Le rendu : les vues sont calculees avec la pose de la tete a cet instant.
		const double lacetRendu = Lacet(temps - kDureeRendu);
		// 2. La tete bouge PENDANT le rendu : a la soumission, elle n'est plus la.
		const double lacetFrais = Lacet(temps);

		// 3. La soumission. JUSTE : les vues employees au rendu, resoumises telles quelles.
		const double soumisJuste = lacetRendu;
		// VERSION FAUTIVE (en commentaire pour la lecture) : redemander les vues
		// juste avant de soumettre, pour l'instant courant.
		//     const double soumisFautif = lacetFrais;
		const double soumis = fraiche ? lacetFrais : soumisJuste;

		// 4. Ce que le compositeur affiche.
		const double apparent = Apparent(lacetFrais, lacetRendu, soumis);
		const double reste = lacetRendu - soumis;   // l'erreur qui reste a l'ecran

		if (rEcran->BeginFrame()) {
			NkICommandBuffer *cmd = rEcran->GetCmd();
			const double a = apparent * kPi / 180.0;
			const NkVec3f pos = { 0.30f, 1.50f, 1.20f };
			const NkVec3f visee = { pos.x + (float32)(std::sin(a) * 3.0), pos.y - 0.45f,
									pos.z - (float32)(std::cos(a) * 3.0) };
			DessinerImage(renderer, rEcran, cmd, pos, visee, (float32)L / (float32)H,
						  (float32)temps, DessinerPoteaux, nullptr);
			if (auto *r2d = rEcran->GetRender2D()) {
				r2d->DrawImage(cible->GetColorResult(), { 0.f, 0.f, (float32)devInit.width, (float32)devInit.height });
			}
			rEcran->Present();
			rEcran->EndFrame();
		}
		++image;

		if (image == imageDeCapture) {
			const char *fichier = fraiche ? "fraiche.png" : "rendu.png";
			const bool ok = cible->Capture(fichier);
			std::printf("[Sim] image %llu : lacet du rendu %.1f, lacet reel %.1f, lacet soumis %.1f, "
						"lacet apparent %.1f, erreur restante %.1f degres -> %s (%s)\n",
						(unsigned long long)image, lacetRendu, lacetFrais, soumis, apparent, reste,
						fichier, ok ? "ok" : "echec");
		}
		if (image >= imageDeCapture + 6) tourne = false;
	}

	NkRenderer::Destroy(renderer);
	NkRenderer::Destroy(rEcran);
	NkDeviceFactory::Destroy(device);
	return 0;
}
