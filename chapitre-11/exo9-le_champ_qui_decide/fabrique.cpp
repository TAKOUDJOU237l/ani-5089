// =============================================================================
//  Exercice 9 (chapitre 11) : le champ qui decide.
//                                                      *** VERSION SIMULEE ***
//
//  Le kit R3D n'a pas de module XR : il n'y a ni runtime OpenXR ni casque ici.
//  Le backend CASQUE ne peut donc pas aboutir, et il SE REPLIE sur le simulateur.
//  Ce repli est le resultat de l'exercice, il n'est pas cache : il est affiche
//  dans la console et dit dans reponse.txt.
//
//  Le choix du backend tient en UN champ du descripteur de session :
//      desc.backend = Backend::SIMULATEUR;     (dans nkmain, une seule ligne)
//  et UN seul test le lit : CreerSession, dans la zone FABRIQUE ci-dessous.
//  La boucle et la scene (le code metier) ne savent pas quel backend tourne.
//
//  Pour la version casque : fabrique_casque.cpp est ce fichier, avec la seule
//  ligne du champ changee (Backend::CASQUE). diff.txt en est la preuve.
//  Capture a l'image 30 : simulateur.png ou casque.png, selon la session.
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

static void ConfigureAppData(NkAppData &d) { d.appName = "SimFabrique"; }
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


// ── Les poteaux : une scene de test, la meme sur tous les backends ──────────
namespace {
void DessinerPoteaux(NkRender3D *r3d, NkMeshSystem *meshes, const void *) {
	Pave(r3d, meshes, { 0.30f, 1.00f, -1.80f }, { 0.08f, 2.00f, 0.08f }, { 0.90f, 0.10f, 0.10f });
	Pave(r3d, meshes, { -0.50f, 1.00f, -1.80f }, { 0.08f, 2.00f, 0.08f }, { 0.10f, 0.30f, 0.90f });
}
} // namespace

// =============================================================================
// FABRIQUE : le descripteur de session, et le SEUL endroit qui lit le backend.
// =============================================================================
namespace {

enum class Backend { SIMULATEUR, CASQUE };

struct DescripteurSession {
	Backend backend = Backend::SIMULATEUR;   // le champ qui decide, pose une fois
};

// Ce que la fabrique rend : des donnees. La boucle les lit, elle ne les teste pas.
struct Session {
	const char *demande = "";       // le backend demande
	const char *obtenu = "";        // le backend obtenu apres repli eventuel
	const char *fichierCapture = "";
	bool repli = false;             // vrai : le backend demande n'a pas pu aboutir
	const char *raison = "";        // pourquoi, quand il y a repli
};

// LE SEUL test du programme qui lit le champ backend.
Session CreerSession(const DescripteurSession &desc) {
	Session s;
	if (desc.backend == Backend::CASQUE) {
		s.demande = "CASQUE";
		s.fichierCapture = "casque.png";
		// Un vrai backend essaierait ici : variable d'environnement, loader standard,
		// decouverte directe. Le kit n'a pas de module XR : rien ne peut aboutir.
		s.obtenu = "SIMULATEUR";
		s.repli = true;
		s.raison = "aucun module XR dans le kit, aucun runtime OpenXR";
	} else {
		s.demande = "SIMULATEUR";
		s.fichierCapture = "simulateur.png";
		s.obtenu = "SIMULATEUR";
	}
	return s;
}

} // namespace

// =============================================================================
// CODE METIER : la boucle. Identique sur les deux backends, elle ne teste rien.
// =============================================================================
int nkmain(const NkEntryState &state) {
	(void)state;

	// ── LA LIGNE DU CHAMP : la seule qui change entre les deux executions ──
	DescripteurSession desc;
	desc.backend = Backend::SIMULATEUR;
	const Session session = CreerSession(desc);

	std::printf("[Fabrique] backend demande : %s, backend obtenu : %s, repli : %s\n",
				session.demande, session.obtenu, session.repli ? "OUI" : "non");
	if (session.repli) {
		std::printf("[Fabrique] raison du repli : %s\n", session.raison);
	}

	NkWindowConfig cfg;
	cfg.title = "Fabrique";
	cfg.width = 1280;
	cfg.height = 720;
	cfg.centered = true;
	cfg.vsync = true;

	NkWindow fenetre(cfg);
	if (!fenetre.IsValid()) { std::printf("[Fabrique] la fenetre n'a pas pu s'ouvrir\n"); return 1; }

	NkDeviceInitInfo devInit{};
	devInit.surface = fenetre.GetSurfaceDesc();
	devInit.width = fenetre.GetSize().x;
	devInit.height = fenetre.GetSize().y;
	NkIDevice *device = NkDeviceFactory::CreateAutoDetect(devInit);
	if (device == nullptr) { std::printf("[Fabrique] aucune API graphique\n"); return 1; }

	NkRendererConfig cfgEcran = NkRendererConfig::For2D(devInit.api, devInit.width, devInit.height);
	NkRenderer *rEcran = NkRenderer::Create(device, cfgEcran);
	if (rEcran == nullptr || !rEcran->Initialize()) { std::printf("[Fabrique] renderer ecran KO\n"); return 1; }

	const uint32 L = 640, H = 360;
	NkOffscreenDesc od;
	od.width = L; od.height = H; od.hasDepth = true; od.hdr = false;
	od.colorFmt = NkGPUFormat::NK_RGBA8_UNORM;
	od.readable = true; od.readback = true; od.name = "Fabrique_Cible";
	NkOffscreenTarget *cible = rEcran->CreateOffscreen(od);
	if (cible == nullptr) { std::printf("[Fabrique] cible refusee\n"); return 1; }

	NkRendererConfig cfgSalle = NkRendererConfig::ForGame(devInit.api, devInit.width, devInit.height);
	cfgSalle.Disable(NK_SS_RENDER2D);
	cfgSalle.Disable(NK_SS_TEXT);
	cfgSalle.Disable(NK_SS_OVERLAY);
	cfgSalle.Enable(NK_SS_OFFSCREEN);
	NkRenderer *renderer = NkRenderer::Create(device, cfgSalle);
	if (renderer == nullptr || !renderer->Initialize()) { std::printf("[Fabrique] renderer salle KO\n"); return 1; }
	renderer->SetFinalColorTarget(rEcran->GetTextures()->GetRHIHandle(cible->GetColorResult()));
	renderer->SetRenderSizeOverride(L, H);

	const uint64 imageDeCapture = 30;   // la meme image pour les deux captures
	uint64 image = 0;
	bool tourne = true;
	while (tourne && fenetre.IsOpen()) {
		NkEvent *ev = nullptr;
		while (NkEvents().PollEvent(ev)) {
			if (ev->As<NkWindowCloseEvent>() != nullptr) tourne = false;
		}
		if (!tourne) break;

		// Pose de la tete scriptee : la meme a chaque image, sur tous les backends.
		const NkVec3f pos = { 0.30f, 1.50f, 1.20f };
		const NkVec3f visee = { pos.x, pos.y - 0.45f, pos.z - 3.0f };

		if (rEcran->BeginFrame()) {
			NkICommandBuffer *cmd = rEcran->GetCmd();
			DessinerImage(renderer, rEcran, cmd, pos, visee, (float32)L / (float32)H,
						  (float32)(image + 1) / 60.f, DessinerPoteaux, nullptr);
			if (auto *r2d = rEcran->GetRender2D()) {
				r2d->DrawImage(cible->GetColorResult(), { 0.f, 0.f, (float32)devInit.width, (float32)devInit.height });
			}
			rEcran->Present();
			rEcran->EndFrame();
		}
		++image;

		if (image == imageDeCapture) {
			const bool ok = cible->Capture(session.fichierCapture);
			std::printf("[Fabrique] image %llu : capture -> %s (%s)\n",
						(unsigned long long)image, session.fichierCapture, ok ? "ok" : "echec");
		}
		if (image >= imageDeCapture + 6) tourne = false;
	}

	NkRenderer::Destroy(renderer);
	NkRenderer::Destroy(rEcran);
	NkDeviceFactory::Destroy(device);
	return 0;
}
