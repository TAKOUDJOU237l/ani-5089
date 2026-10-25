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

#include "NKImage/Core/NkImage.h"
#include "NKMath/NkColor.h"

#include <cstdio>
#include <cstdlib>

using namespace nkentseu;
using namespace nkentseu::renderer;

static void ConfigureAppData(NkAppData &d) { d.appName = "MaSalle"; }
NK_REGISTER_ENTRY_APPDATA_UPDATER(ConfigureAppData)

namespace {

	// ── Les dimensions de la salle (identiques a etape5_salle.cpp) ─────────
	const float32 kPiece = 4.00f;
	const float32 kPlafond = 2.50f;
	const float32 kMur = 0.10f;
	const float32 kPorteL = 0.90f;
	const float32 kPorteH = 2.00f;
	const float32 kFenL = 1.20f;
	const float32 kFenH = 1.00f;
	const float32 kFenY = 1.40f;
	const float32 kTableL = 1.20f;
	const float32 kTableP = 0.80f;
	const float32 kTableH = 0.75f;
	const float32 kYeux = 1.70f;

	const float32 kDemi = kPiece * 0.5f;

	// ── La tete et les yeux ─────────────────────────────────────────────────
	const float32 kTeteX = 0.30f;
	const float32 kTeteZ = 1.20f;
	const float32 kEcartYeux = 0.063f;				// 63 mm
	const float32 kDemiEcart = kEcartYeux * 0.5f;	// 0,0315 m

	// ── La taille d'un oeil ─────────────────────────────────────────────────
	const uint32 kLargeurOeil = 640;
	const uint32 kHauteurOeil = 720;

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

		{ // le sol : le plan est unitaire, une echelle S donne un cote S
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

		Pave(r3d, meshes, { 0.f, yMur, -bord }, { kPiece, kPlafond, kMur }, grisMur);
		Pave(r3d, meshes, { 0.f, yMur, bord }, { kPiece, kPlafond, kMur }, grisMur);
		Pave(r3d, meshes, { -bord, yMur, 0.f }, { kMur, kPlafond, kPiece }, grisMur);
		Pave(r3d, meshes, { bord, yMur, 0.f }, { kMur, kPlafond, kPiece }, grisMur);

		Pave(r3d, meshes, { 0.f, kPlafond + kMur * 0.5f, 0.f },
			 { kPiece, kMur, kPiece }, grisMur);

		Pave(r3d, meshes, { -0.70f, kPorteH * 0.5f, -kDemi + 0.02f },
			 { kPorteL, kPorteH, 0.04f }, bois, 0.6f);

		Pave(r3d, meshes, { -kDemi + 0.02f, kFenY, -0.90f },
			 { 0.04f, kFenH, kFenL }, vitre, 0.15f);
		Pave(r3d, meshes, { -kDemi + 0.02f, kFenY, 0.90f },
			 { 0.04f, kFenH, kFenL }, vitre, 0.15f);

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

		Pave(r3d, meshes, { -1.10f, 0.50f, 1.20f }, { 1.f, 1.f, 1.f },
			 { 0.85f, 0.80f, 0.30f }, 0.8f);
	}

	// ── Dessiner la salle, vue d'un oeil place en `pos` ────────────────────
	// Le renderer dessine dans SA cible ; son graphe s'execute sur le tampon de commandes de l'ecran. Tout ce qui
	// n'est pas la position est identique d'un oeil a l'autre : orientation (regard -Z, haut +Y), champ de vision,
	// plans, lumiere, salle.
	void DessinerOeil(NkRenderer *rOeil, NkICommandBuffer *cmd, const NkVec3f &pos, uint64 image) {
		NkRender3D *r3d = rOeil->GetRender3D();

		// Ce que BeginFrame ferait si ce renderer possedait l'image. Chaque oeil le rejoue pour LUI-MEME : sans
		// ResetFrame, le second oeil herite de l'etat du premier.
		rOeil->FlushGraphRebuilds();
		r3d->ResetFrame();
		if (auto *mc = rOeil->GetMaterialCollection()) {
			mc->Upload();
		}

		NkCamera3DData cd;
		cd.position = pos;
		cd.target = { pos.x, pos.y, pos.z - 1.f }; // meme orientation : tete droite, regard -Z
		cd.up = { 0.f, 1.f, 0.f };
		cd.fovY = 70.f;
		cd.aspect = (float32)kLargeurOeil / (float32)kHauteurOeil;
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
		SoumettreLaSalle(r3d, rOeil->GetMeshSystem());

		if (auto *graph = rOeil->GetRenderGraph()) {
			graph->Execute(cmd);
		}
	}

	// ── La comparaison, faite par le programme ──────────────────────────────
	struct Difference {
		long long pixelsDifferents = 0; // pixels dont l'ecart depasse 1
		long long total = 0;			// nombre de pixels compares
		int ecartMax = 0;
		bool ok = false;
	};

	int AbsInt(int v) { return v < 0 ? -v : v; }

	int Max3i(int a, int b, int c) {
		int m = a > b ? a : b;
		return m > c ? m : c;
	}

	// Compare deux fichiers image pixel par pixel et ecrit l'image de la difference.
	Difference Comparer(const char *fichierA, const char *fichierB, const char *sortiePng) {
		Difference r;
		NkImage a, b;
		if (!a.Load(fichierA, 4) || !b.Load(fichierB, 4) || !a.IsValid() || !b.IsValid()) {
			std::printf("Comparer : impossible de relire %s ou %s\n", fichierA, fichierB);
			return r;
		}
		const int largeur = a.Width() < b.Width() ? a.Width() : b.Width();
		const int hauteur = a.Height() < b.Height() ? a.Height() : b.Height();
		NkImage diff = NkImage::Create((uint32)largeur, (uint32)hauteur, 4);
		for (int y = 0; y < hauteur; ++y) {
			for (int x = 0; x < largeur; ++x) {
				const auto pa = a.GetPixel(x, y);
				const auto pb = b.GetPixel(x, y);
				const int ecart = Max3i(AbsInt((int)pa.r - (int)pb.r), AbsInt((int)pa.g - (int)pb.g),
										AbsInt((int)pa.b - (int)pb.b));
				if (ecart > 1)
					++r.pixelsDifferents;
				if (ecart > r.ecartMax)
					r.ecartMax = ecart;
				++r.total;
				int v = 4 * ecart; // quatre fois l'ecart, borne a 255
				if (v > 255)
					v = 255;
				diff.SetPixel(x, y, math::NkColor((uint8)v, (uint8)v, (uint8)v, 255));
			}
		}
		diff.SavePNG(sortiePng);
		r.ok = true;
		return r;
	}

	void AfficherLigne(const char *nom, const Difference &d) {
		std::printf("%s : %lld pixels sur %lld, ecart max %d\n", nom, d.pixelsDifferents, d.total, d.ecartMax);
	}

} // namespace

int nkmain(const NkEntryState &state) {
	(void)state;

	// ── Mode comparaison seule : relit les quatre PNG, ecrit difference.txt ─
	const char *comparer = std::getenv("MASALLE_COMPARER");
	if (comparer && comparer[0] == '1') {
		const Difference deux = Comparer("oeil-gauche.png", "oeil-droit.png", "difference.png");
		const Difference oubli = Comparer("oubli-gauche.png", "oubli-droit.png", "difference-vue-oubliee.png");
		FILE *f = std::fopen("difference.txt", "w");
		if (deux.ok) {
			AfficherLigne("deux yeux", deux);
			if (f)
				std::fprintf(f, "deux yeux : %lld pixels sur %lld, ecart max %d\n", deux.pixelsDifferents, deux.total,
							 deux.ecartMax);
		}
		if (oubli.ok) {
			AfficherLigne("vue oubliee", oubli);
			if (f)
				std::fprintf(f, "vue oubliee : %lld pixels sur %lld, ecart max %d\n", oubli.pixelsDifferents,
							 oubli.total, oubli.ecartMax);
		}
		if (f)
			std::fclose(f);
		return (deux.ok && oubli.ok) ? 0 : 1;
	}

	const char *oubliEnv = std::getenv("MASALLE_OUBLI");
	const bool oubli = (oubliEnv != nullptr && oubliEnv[0] == '1');

	// Les positions des deux yeux. Vue normale : decalage de 31,5 mm de part et d'autre de la tete, le long de sa
	// droite (+X). Vue oubliee : le decalage est oublie, les deux cameras sont a la place de la tete.
	const NkVec3f tete = { kTeteX, kYeux, kTeteZ };
	NkVec3f posGauche = { tete.x - kDemiEcart, tete.y, tete.z };
	NkVec3f posDroit = { tete.x + kDemiEcart, tete.y, tete.z };
	if (oubli) {
		posGauche = tete;
		posDroit = tete;
	}

	NkWindowConfig cfg;
	cfg.title = oubli ? "Deux yeux - vue oubliee" : "Deux yeux";
	cfg.width = 1280;
	cfg.height = 720;
	cfg.centered = true;
	cfg.vsync = true;

	NkWindow fenetre(cfg);
	if (!fenetre.IsValid()) {
		std::printf("[DeuxYeux] la fenetre n'a pas pu s'ouvrir\n");
		return 1;
	}

	NkDeviceInitInfo devInit{};
	devInit.surface = fenetre.GetSurfaceDesc();
	devInit.width = fenetre.GetSize().x;
	devInit.height = fenetre.GetSize().y;

	NkIDevice *device = NkDeviceFactory::CreateAutoDetect(devInit);
	if (device == nullptr) {
		std::printf("[DeuxYeux] aucune API graphique disponible\n");
		return 1;
	}

	// ── Le renderer d'ecran : il POSSEDE l'image (BeginFrame / Present / EndFrame), il ne dessine que du 2D ──
	NkRendererConfig cfgEcran = NkRendererConfig::For2D(devInit.api, devInit.width, devInit.height);
	NkRenderer *rEcran = NkRenderer::Create(device, cfgEcran);
	if (rEcran == nullptr || !rEcran->Initialize()) {
		std::printf("[DeuxYeux] le renderer d'ecran n'a pas pu s'initialiser\n");
		NkDeviceFactory::Destroy(device);
		return 1;
	}

	// ── Deux cibles hors ecran de 640 x 720, une par oeil, relisibles ──────
	NkOffscreenDesc od;
	od.width = kLargeurOeil;
	od.height = kHauteurOeil;
	od.hasDepth = true;
	od.hdr = false;
	od.colorFmt = NkGPUFormat::NK_RGBA8_UNORM;
	od.readable = true;
	od.readback = true; // sans quoi Capture() ne peut rien relire

	od.name = "DeuxYeux_Gauche";
	NkOffscreenTarget *cibleG = rEcran->CreateOffscreen(od);
	od.name = "DeuxYeux_Droit";
	NkOffscreenTarget *cibleD = rEcran->CreateOffscreen(od);
	if (cibleG == nullptr || cibleD == nullptr) {
		std::printf("[DeuxYeux] cible hors ecran refusee\n");
		NkRenderer::Destroy(rEcran);
		NkDeviceFactory::Destroy(device);
		return 1;
	}

	// ── Deux renderers de salle, un par oeil : le meme reglage, chacun SA cible ──
	// SetFinalColorTarget reconstruit le graphe : on le fait UNE fois, ici, jamais dans la boucle.
	NkRendererConfig cfgSalle = NkRendererConfig::ForGame(devInit.api, devInit.width, devInit.height);
	cfgSalle.Disable(NK_SS_RENDER2D);
	cfgSalle.Disable(NK_SS_TEXT);
	cfgSalle.Disable(NK_SS_OVERLAY);
	cfgSalle.Enable(NK_SS_OFFSCREEN);

	NkRenderer *rGauche = NkRenderer::Create(device, cfgSalle);
	NkRenderer *rDroit = NkRenderer::Create(device, cfgSalle);
	if (rGauche == nullptr || !rGauche->Initialize() || rDroit == nullptr || !rDroit->Initialize()) {
		std::printf("[DeuxYeux] un renderer de salle n'a pas pu s'initialiser\n");
		NkRenderer::Destroy(rDroit);
		NkRenderer::Destroy(rGauche);
		NkRenderer::Destroy(rEcran);
		NkDeviceFactory::Destroy(device);
		return 1;
	}
	rGauche->SetFinalColorTarget(rEcran->GetTextures()->GetRHIHandle(cibleG->GetColorResult()));
	rGauche->SetRenderSizeOverride(kLargeurOeil, kHauteurOeil);
	rDroit->SetFinalColorTarget(rEcran->GetTextures()->GetRHIHandle(cibleD->GetColorResult()));
	rDroit->SetRenderSizeOverride(kLargeurOeil, kHauteurOeil);

	std::printf("[DeuxYeux] mode %s : oeil gauche (%.4f, %.4f, %.4f), oeil droit (%.4f, %.4f, %.4f)\n",
				oubli ? "vue oubliee" : "deux yeux", (double)posGauche.x, (double)posGauche.y, (double)posGauche.z,
				(double)posDroit.x, (double)posDroit.y, (double)posDroit.z);

	// Les deux captures sont prises a la MEME image : l'image 30, apres que les deux yeux ont ete dessines.
	const uint64 imageDeCapture = 30;
	const char *nomGauche = oubli ? "oubli-gauche.png" : "oeil-gauche.png";
	const char *nomDroit = oubli ? "oubli-droit.png" : "oeil-droit.png";
	const char *nomDiff = oubli ? "difference-vue-oubliee.png" : "difference.png";

	uint64 image = 0;
	bool tourne = true;
	bool captureFaite = false;

	while (tourne && fenetre.IsOpen()) {
		NkEvent *ev = nullptr;
		while (NkEvents().PollEvent(ev)) {
			if (ev->As<NkWindowCloseEvent>() != nullptr) {
				tourne = false;
			}
		}
		if (!tourne)
			break;

		if (rEcran->BeginFrame()) {
			NkICommandBuffer *cmd = rEcran->GetCmd();

			// Oeil gauche puis oeil droit : la meme salle, deux positions.
			DessinerOeil(rGauche, cmd, posGauche, image);
			DessinerOeil(rDroit, cmd, posDroit, image);

			// Les deux images, cote a cote dans la fenetre : gauche a x = 0, droite a x = 640.
			if (auto *r2d = rEcran->GetRender2D()) {
				r2d->DrawImage(cibleG->GetColorResult(), { 0.f, 0.f, (float32)kLargeurOeil, (float32)kHauteurOeil });
				r2d->DrawImage(cibleD->GetColorResult(),
							   { (float32)kLargeurOeil, 0.f, (float32)kLargeurOeil, (float32)kHauteurOeil });
			}

			rEcran->Present();
			rEcran->EndFrame();
		}

		++image;

		if (image == imageDeCapture && !captureFaite) {
			const bool okG = cibleG->Capture(nomGauche);
			const bool okD = cibleD->Capture(nomDroit);
			std::printf("[DeuxYeux] capture image %llu : %s (%s), %s (%s)\n", (unsigned long long)image, nomGauche,
						okG ? "ok" : "echec", nomDroit, okD ? "ok" : "echec");
			captureFaite = true;
			if (okG && okD) {
				const Difference d = Comparer(nomGauche, nomDroit, nomDiff);
				if (d.ok) {
					AfficherLigne(oubli ? "vue oubliee" : "deux yeux", d);
					std::printf("[DeuxYeux] image de difference : %s\n", nomDiff);
				}
			}
			tourne = false; // tout est capture : on s'arrete proprement
		}
	}

	std::printf("[DeuxYeux] %llu images\n", (unsigned long long)image);

	NkRenderer::Destroy(rDroit);
	NkRenderer::Destroy(rGauche);
	NkRenderer::Destroy(rEcran);
	NkDeviceFactory::Destroy(device);
	return captureFaite ? 0 : 2;
}