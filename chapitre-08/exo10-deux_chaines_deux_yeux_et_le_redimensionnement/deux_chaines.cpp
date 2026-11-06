// =============================================================================
//  Exercice 10 : deux chaines d'echange, une par oeil, et le redimensionnement.
//
//  Chaque oeil a sa chaine (ChaineEchange, celle de l'exercice 8), son anneau
//  de 3 images, son renderer et sa cible hors ecran. A chaque image, pour
//  chaque oeil : les quatre temps (emprunter, dessiner, rendre, soumettre).
//
//  Les poignees enregistrees dans la chaine sont des entiers 64 bits OPAQUES :
//  ici, l'adresse de la cible hors ecran de l'oeil. Le simulateur n'a qu'UNE
//  vraie image par oeil : l'anneau (3 index) tient la discipline, les 3 poignees
//  pointent la meme cible. La soumission relit la poignee de l'index rendu : si
//  elle est a zero (chaine refaite mais images non reenregistrees), l'oeil n'est
//  pas affiche, ecran noir de ce cote.
//
//  Au redimensionnement de la fenetre (NkWindowResizeEvent), au debut de l'image
//  suivante, dans cet ordre, une operation par ligne dans resize.txt :
//      detruire les deux chaines, detruire les deux cibles, recreer les deux
//      cibles a la nouvelle taille, recreer les deux chaines, reenregistrer
//      leurs images.
//
//  Fichiers ecrits dans le dossier de lancement :
//      trace.txt  : "image : N, gauche : i, droit : j" pour les 30 premieres
//                   images, puis pour les 10 images qui suivent un redimensionnement.
//      resize.txt : ce qui a ete detruit / recree / reenregistre.
//  Variables : DC_IMAGES (images avant arret, 1200 par defaut, ~20 s), DC_DEUX_SANS_REENREGISTRER=1
//  pour omettre le reenregistrement (observer l'oeil noir).
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

#include <cstdio>
#include <cstdlib>
#include <cstdint>

using namespace nkentseu;
using namespace nkentseu::renderer;

static void ConfigureAppData(NkAppData &d) { d.appName = "MaSalle"; }
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

// Chaine d'echange de l'exercice 8 : un anneau, un drapeau d'emprunt, des
// poignees 64 bits sans type.
class ChaineEchange {
public:
	static constexpr uint32 kMaxImages = 4;
	bool Creer(uint32 n) {
		if (n == 0u || n > kMaxImages) return false;
		mCount = n; mNext = 0u; mAcquired = false;
		for (uint32 i = 0u; i < kMaxImages; ++i) mHandles[i] = 0u;
		return true;
	}
	void Detruire() { mCount = 0u; mNext = 0u; mAcquired = false; for (uint32 i = 0u; i < kMaxImages; ++i) mHandles[i] = 0u; }
	bool AcquireImage(uint32 &index) {
		if (mCount == 0u || mAcquired) return false;
		mAcquired = true; index = mNext; return true;
	}
	bool ReleaseImage() {
		if (!mAcquired) return false;
		mAcquired = false; mNext = (mNext + 1u) % mCount; return true;
	}
	uint32 GetLastReleasedIndex() const { return (mNext + mCount - 1u) % mCount; }
	bool SetHandle(uint32 i, uint64_t h) { if (i >= mCount) return false; mHandles[i] = h; return true; }
	uint64_t GetHandle(uint32 i) const { return i < mCount ? mHandles[i] : 0u; }
	uint32 Count() const { return mCount; }
private:
	uint64_t mHandles[kMaxImages] = {};
	uint32 mCount = 0u, mNext = 0u;
	bool mAcquired = false;
};

namespace {

	const uint32 kImagesParChaine = 3u;

	// Un oeil : sa chaine, son renderer 3D, sa cible hors ecran.
	struct Oeil {
		const char *nom = "";			 // "gauche" / "droit"
		float32 decalageX = 0.f;		 // ecart de la camera, en metres
		NkRenderer *renderer = nullptr;
		NkOffscreenTarget *cible = nullptr;
		ChaineEchange chaine;
	};

	NkOffscreenTarget *CreerCible(NkRenderer *rEcran, uint32 w, uint32 h, const char *nom) {
		NkOffscreenDesc od;
		od.width = w;
		od.height = h;
		od.hasDepth = true;
		od.hdr = false;
		od.colorFmt = NkGPUFormat::NK_RGBA8_UNORM;
		od.readable = true;
		od.readback = false;
		od.name = nom;
		return rEcran->CreateOffscreen(od);
	}

	// Relie un renderer a sa cible : DEUX appels, pas un (exercice 9).
	void RelierCible(NkRenderer *rEcran, Oeil &o, uint32 w, uint32 h) {
		o.renderer->SetFinalColorTarget(
			rEcran->GetTextures()->GetRHIHandle(o.cible->GetColorResult()));
		o.renderer->SetRenderSizeOverride(w, h);
	}

	// Enregistre les images de la chaine : une poignee opaque par index.
	void Reenregistrer(Oeil &o) {
		for (uint32 i = 0u; i < o.chaine.Count(); ++i) {
			o.chaine.SetHandle(i, (uint64_t)(uintptr_t)o.cible);
		}
	}

	// Dessine la salle pour un oeil, dans sa cible (temps 2 des quatre).
	void DessinerOeil(Oeil &o, NkRenderer *rEcran, uint32 w, uint32 h, uint64 image) {
		NkICommandBuffer *cmd = rEcran->GetCmd();
		NkRender3D *r3d = o.renderer->GetRender3D();
		o.renderer->FlushGraphRebuilds();
		r3d->ResetFrame();
		if (auto *mc = o.renderer->GetMaterialCollection()) mc->Upload();

		NkCamera3DData cd;
		cd.position = { 0.30f + o.decalageX, kYeux, 1.20f };
		cd.target = { -0.70f + o.decalageX, 1.10f, -kDemi };
		cd.up = { 0.f, 1.f, 0.f };
		cd.fovY = 70.f;
		cd.aspect = (float32)w / (float32)h;
		cd.nearPlane = 0.05f;
		cd.farPlane = 100.f;

		NkSceneContext sctx;
		sctx.camera = NkCamera3D(cd);
		sctx.time = (float32)image / 60.f;
		r3d->SetIBLStrength(0.35f);

		NkLightDesc soleil;
		soleil.type = NkLightType::NK_DIRECTIONAL;
		soleil.direction = { -0.5f, -1.f, -0.35f };
		soleil.color = { 1.f, 0.96f, 0.88f };
		soleil.intensity = 3.f;
		soleil.castShadow = false;
		sctx.lights.PushBack(soleil);

		r3d->BeginScene(sctx);
		SoumettreLaSalle(r3d, o.renderer->GetMeshSystem());
		if (auto *graph = o.renderer->GetRenderGraph()) graph->Execute(cmd);
	}

} // namespace

int nkmain(const NkEntryState &state) {
	(void)state;

	NkWindowConfig cfg;
	cfg.title = "Deux chaines";
	cfg.width = 1280;
	cfg.height = 720;
	cfg.centered = true;
	cfg.vsync = true;

	NkWindow fenetre(cfg);
	if (!fenetre.IsValid()) {
		std::printf("[DeuxChaines] la fenetre n'a pas pu s'ouvrir\n");
		return 1;
	}

	NkDeviceInitInfo devInit{};
	devInit.surface = fenetre.GetSurfaceDesc();
	devInit.width = fenetre.GetSize().x;
	devInit.height = fenetre.GetSize().y;

	NkIDevice *device = NkDeviceFactory::CreateAutoDetect(devInit);
	if (device == nullptr) {
		std::printf("[DeuxChaines] aucune API graphique disponible\n");
		return 1;
	}

	NkRendererConfig cfgEcran = NkRendererConfig::For2D(devInit.api, devInit.width, devInit.height);
	NkRenderer *rEcran = NkRenderer::Create(device, cfgEcran);
	if (rEcran == nullptr || !rEcran->Initialize()) {
		std::printf("[DeuxChaines] le renderer d'ecran n'a pas pu s'initialiser\n");
		NkDeviceFactory::Destroy(device);
		return 1;
	}

	uint32 winW = (uint32)devInit.width, winH = (uint32)devInit.height;
	uint32 cibleW = winW / 2u, cibleH = winH;

	Oeil yeux[2];
	yeux[0].nom = "gauche"; yeux[0].decalageX = -0.032f;
	yeux[1].nom = "droit";  yeux[1].decalageX = +0.032f;

	for (int k = 0; k < 2; ++k) {
		Oeil &o = yeux[k];
		o.cible = CreerCible(rEcran, cibleW, cibleH, k == 0 ? "Oeil_gauche" : "Oeil_droit");
		if (o.cible == nullptr) {
			std::printf("[DeuxChaines] cible %s refusee\n", o.nom);
			return 1;
		}
		NkRendererConfig cfgSalle = NkRendererConfig::ForGame(devInit.api, cibleW, cibleH);
		cfgSalle.Disable(NK_SS_RENDER2D);
		cfgSalle.Disable(NK_SS_TEXT);
		cfgSalle.Disable(NK_SS_OVERLAY);
		cfgSalle.Enable(NK_SS_OFFSCREEN);
		o.renderer = NkRenderer::Create(device, cfgSalle);
		if (o.renderer == nullptr || !o.renderer->Initialize()) {
			std::printf("[DeuxChaines] renderer %s non initialise\n", o.nom);
			return 1;
		}
		RelierCible(rEcran, o, cibleW, cibleH);
		o.chaine.Creer(kImagesParChaine);
		Reenregistrer(o);
	}

	std::FILE *trace = std::fopen("trace.txt", "w");
	std::FILE *resize = std::fopen("resize.txt", "w");
	const bool sansReenregistrer = EnvU64("DC_DEUX_SANS_REENREGISTRER", 0) != 0;
	const uint64 imageDeSortie = EnvU64("DC_IMAGES", 1200);

	uint64 image = 0;
	int imagesApresResize = 1000;   // grand : pas de trace tant qu'on n'a pas redimensionne
	bool tourne = true;
	bool resizeEnAttente = false;
	uint32 nouvelleW = 0u, nouvelleH = 0u;

	while (tourne && fenetre.IsOpen()) {
		NkEvent *ev = nullptr;
		while (NkEvents().PollEvent(ev)) {
			if (ev->As<NkWindowCloseEvent>() != nullptr) {
				tourne = false;
			} else if (auto *re = ev->As<NkWindowResizeEvent>()) {
				const uint32 w = re->GetWidth(), h = re->GetHeight();
				if (w >= 2u && h >= 1u && (w != winW || h != winH)) {
					resizeEnAttente = true;
					nouvelleW = w;
					nouvelleH = h;
				}
			}
		}
		if (!tourne) break;

		// ── Le redimensionnement, au debut d'une image, pas au milieu ─────────
		if (resizeEnAttente) {
			resizeEnAttente = false;
			winW = nouvelleW; winH = nouvelleH;
			cibleW = winW / 2u; cibleH = winH;
			auto op = [&](const char *texte) {
				if (resize) { std::fprintf(resize, "%s\n", texte); std::fflush(resize); }
				std::printf("[DeuxChaines] %s\n", texte);
			};
			char ligne[160];
			std::snprintf(ligne, sizeof(ligne), "fenetre redimensionnee : %u x %u", winW, winH);
			op(ligne);
			for (int k = 0; k < 2; ++k) { yeux[k].chaine.Detruire(); std::snprintf(ligne, sizeof(ligne), "detruit la chaine %s", yeux[k].nom); op(ligne); }
			for (int k = 0; k < 2; ++k) { rEcran->DestroyOffscreen(yeux[k].cible); std::snprintf(ligne, sizeof(ligne), "detruit la cible %s", yeux[k].nom); op(ligne); }
			rEcran->OnResize(winW, winH);
			op("redimensionne le renderer d'ecran");
			for (int k = 0; k < 2; ++k) {
				yeux[k].cible = CreerCible(rEcran, cibleW, cibleH, k == 0 ? "Oeil_gauche" : "Oeil_droit");
				if (yeux[k].cible == nullptr) { std::printf("[DeuxChaines] cible refusee apres redimensionnement\n"); tourne = false; break; }
				RelierCible(rEcran, yeux[k], cibleW, cibleH);
				std::snprintf(ligne, sizeof(ligne), "recree la cible %s en %u x %u", yeux[k].nom, cibleW, cibleH); op(ligne);
			}
			if (!tourne) break;
			for (int k = 0; k < 2; ++k) { yeux[k].chaine.Creer(kImagesParChaine); std::snprintf(ligne, sizeof(ligne), "recree la chaine %s", yeux[k].nom); op(ligne); }
			if (!sansReenregistrer) {
				for (int k = 0; k < 2; ++k) { Reenregistrer(yeux[k]); std::snprintf(ligne, sizeof(ligne), "reenregistre les images de la chaine %s", yeux[k].nom); op(ligne); }
			} else {
				op("reenregistrement OMIS (DC_DEUX_SANS_REENREGISTRER=1)");
			}
			imagesApresResize = 0;
		}

		int soumis[2] = { -1, -1 };
		if (rEcran->BeginFrame()) {
			for (int k = 0; k < 2; ++k) {
				Oeil &o = yeux[k];
				uint32 index = 0u;
				if (!o.chaine.AcquireImage(index)) continue;         // 1. emprunter
				DessinerOeil(o, rEcran, cibleW, cibleH, image);       // 2. dessiner
				if (!o.chaine.ReleaseImage()) continue;               // 3. rendre
				soumis[k] = (int)o.chaine.GetLastReleasedIndex();     // 4. soumettre : l'index RENDU
			}
			// 4 (suite) : l'ecran recopie, pour chaque oeil, l'image que la chaine
			// designe. Poignee a zero : rien n'est affiche de ce cote.
			if (auto *r2d = rEcran->GetRender2D()) {
				for (int k = 0; k < 2; ++k) {
					if (soumis[k] < 0) continue;
					const uint64_t h = yeux[k].chaine.GetHandle((uint32)soumis[k]);
					if (h == 0u) continue;
					NkOffscreenTarget *t = (NkOffscreenTarget *)(uintptr_t)h;
					r2d->DrawImage(t->GetColorResult(),
								   { k == 0 ? 0.f : (float32)cibleW, 0.f, (float32)cibleW, (float32)cibleH });
				}
			}
			rEcran->Present();
			rEcran->EndFrame();
		}

		++image;
		++imagesApresResize;
		if (trace != nullptr && soumis[0] >= 0 && soumis[1] >= 0 &&
			(image <= 30 || imagesApresResize <= 10)) {
			std::fprintf(trace, "image : %llu, gauche : %d, droit : %d\n",
						 (unsigned long long)image, soumis[0], soumis[1]);
			std::fflush(trace);
		}
		if (imageDeSortie != 0 && image >= imageDeSortie) tourne = false;
	}

	std::printf("[DeuxChaines] %llu images\n", (unsigned long long)image);
	if (trace) std::fclose(trace);
	if (resize) std::fclose(resize);

	NkRenderer::Destroy(yeux[1].renderer);
	NkRenderer::Destroy(yeux[0].renderer);
	NkRenderer::Destroy(rEcran);
	NkDeviceFactory::Destroy(device);
	return 0;
}