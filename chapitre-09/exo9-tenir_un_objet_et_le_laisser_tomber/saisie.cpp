// =============================================================================
//  Exercice 9 (Sprint 9) : saisir, front contre niveau.   *** VERSION SIMULEE ***
//
//  Le kit du moteur n'a pas de module d'actions XR. La "gachette" est donc un
//  script (presse entre les images 40 et 100) et la paume une trajectoire
//  calculee : courant / change / actif sont recalcules comme le chapitre les
//  definit, et la logique de saisie est celle qu'on ecrirait avec l'API reelle.
//  Rendu, camera et capture sont ceux du moteur.
//
//    (rien)          : version JUSTE   - tenu tant que courant est vrai
//    SAISIE_FAUTIF=1 : version FAUTIVE - tenu seulement sur le front (change)
//  Capture a l'image 46 : niveau.png (juste) ou front.png (fautive).
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

static void ConfigureAppData(NkAppData &d) { d.appName = "SimXR"; }
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

// ── Logique simulee (sans module XR) ──
namespace {
// ── Etat d'une action booleenne : les trois champs du chapitre ───────────────
struct EtatAction {
	bool courant = false;   // l'action est vraie a cette image
	bool change = false;    // courant a bascule depuis la synchronisation precedente
	bool actif = false;     // une source materielle est liee
};

// Action simulee : sans module XR, la "source materielle" est un script.
class ActionBool {
public:
	void Synchroniser(bool presse, bool lie) {
		EtatAction suivant;
		suivant.actif = lie;
		suivant.courant = lie ? presse : false;      // inactive => courant faux
		suivant.change = suivant.courant != mEtat.courant;   // calcule sur courant
		mEtat = suivant;
	}
	const EtatAction &Etat() const { return mEtat; }
private:
	EtatAction mEtat;
};

// ── Pose simulee : position + lacet (rotation autour de l'axe vertical) ─────
struct PoseSim { double x = 0, y = 0, z = 0, lacet = 0; };

// Rotation d'un vecteur (x, z) de `a` radians autour de l'axe vertical.
static void Tourner(double a, double x, double z, double &ox, double &oz) {
	ox = x * std::cos(a) + z * std::sin(a);
	oz = -x * std::sin(a) + z * std::cos(a);
}
// a * b : la pose b exprimee dans le repere de a.
static PoseSim Composer(const PoseSim &a, const PoseSim &b) {
	PoseSim r;
	double dx, dz;
	Tourner(a.lacet, b.x, b.z, dx, dz);
	r.x = a.x + dx; r.y = a.y + b.y; r.z = a.z + dz;
	r.lacet = a.lacet + b.lacet;
	return r;
}
static PoseSim Inverser(const PoseSim &a) {
	PoseSim r;
	double dx, dz;
	Tourner(-a.lacet, a.x, a.z, dx, dz);
	r.x = -dx; r.y = -a.y; r.z = -dz;
	r.lacet = -a.lacet;
	return r;
}
static double Distance(const PoseSim &a, const PoseSim &b) {
	return std::sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y) + (a.z - b.z) * (a.z - b.z));
}
static double Lerp(double a, double b, double u) { return a + (b - a) * u; }

// La paume simulee : de (x0,y0,z0) a (x1,y1,z1) entre les images i0 et i1,
// le lacet oscille doucement pour que la composition de poses serve vraiment.
static PoseSim PaumeSim(int image, int i0, int i1, const PoseSim &debut, const PoseSim &fin) {
	double u = (double)(image - i0) / (double)(i1 - i0);
	if (u < 0) u = 0;
	if (u > 1) u = 1;
	PoseSim p;
	p.x = Lerp(debut.x, fin.x, u); p.y = Lerp(debut.y, fin.y, u); p.z = Lerp(debut.z, fin.z, u);
	p.lacet = 0.5 * std::sin(image * 0.06);
	return p;
}

// L'objet : au repos sur la table, tenu par la main, ou en chute libre.
struct Objet {
	PoseSim pose{0.60, 0.825, 0.70, 0.0};
	PoseSim attache;         // pose de l'objet DANS le repere de la paume, figee a la saisie
	bool tenu = false;
	bool enChute = false;
	double vy = 0.0;
	void Saisir(const PoseSim &paume) {
		attache = Composer(Inverser(paume), pose);   // une fois, a la saisie
		tenu = true; enChute = false;
	}
	void Lacher() { tenu = false; enChute = true; vy = 0.0; }
	void Avancer(const PoseSim &paume) {
		const double dt = 1.0 / 60.0;
		if (tenu) {
			pose = Composer(paume, attache);         // a chaque image
		} else if (enChute) {
			vy += 9.8 * dt;
			pose.y -= vy * dt;
			// le support : le plateau (0,75 m) s'il est sous l'objet, sinon le sol
			const bool surTable = std::fabs(pose.x - 0.60) < 0.60 && std::fabs(pose.z - 0.70) < 0.40;
			const double support = (surTable ? 0.75 : 0.0) + 0.075;
			if (pose.y <= support) { pose.y = support; enChute = false; vy = 0.0; }
		}
	}
};

// ── Exercice 9 : saisir, front contre niveau ─────────────────────────────────
struct Monde {
	ActionBool saisir;
	Objet objet;
	PoseSim paume;
	bool fautif = false;    // true : version fautive, on ne tient que sur le front

	void Avancer(int image) {
		paume = (image <= 40) ? PaumeSim(image, 0, 40, PoseSim{-0.30, 1.00, 0.70, 0}, PoseSim{0.60, 0.90, 0.70, 0})
							  : PaumeSim(image, 40, 100, PoseSim{0.60, 0.90, 0.70, 0}, PoseSim{-0.20, 1.35, 0.50, 0});
		const bool presse = image >= 40 && image <= 100;   // gachette de saisie
		saisir.Synchroniser(presse, true);
		const EtatAction &e = saisir.Etat();
		const bool front = e.courant && e.change;         // saisie : current ET changed

		if (front && !objet.tenu && Distance(paume, objet.pose) < 0.20) {
			objet.Saisir(paume);                            // attache a la PAUME, pas a la visee
		}
		// VERSION FAUTIVE (a garder en commentaire pour la lecture) : l'objet
		// n'est tenu que le temps du front, donc il tombe a l'image suivante.
		//     if (objet.tenu && !front) objet.Lacher();
		// VERSION JUSTE : il est tenu tant que le niveau est vrai, et on ne lache
		// que sur un VRAI front de relachement : source presente (actif), niveau
		// retombe (!courant) et bascule a cette image (change).
		//     if (objet.tenu && e.actif && !e.courant && e.change) objet.Lacher();
		if (fautif) {
			if (objet.tenu && !front) objet.Lacher();
		} else {
			if (objet.tenu && e.actif && !e.courant && e.change) objet.Lacher();
		}
		objet.Avancer(paume);
	}
};

} // namespace

int nkmain(const NkEntryState &state) {
	(void)state;

	NkWindowConfig cfg;
	cfg.title = "Simulation XR";
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

	Monde monde;
	const bool fautif = EnvU64("SAISIE_FAUTIF", 0) != 0; monde.fautif = fautif;
	std::printf("[Sim] SIMULATION : aucune action XR du moteur, la source est un script. Mode : %s\n", (fautif ? "FAUTIF (front seul)" : "JUSTE (niveau)"));
	std::FILE *journal = std::fopen("journal-saisie.txt", "w");

	const uint64 imageDeSortie = 60;
	uint64 image = 0;
	bool tourne = true;
	while (tourne && fenetre.IsOpen()) {
		NkEvent *ev = nullptr;
		while (NkEvents().PollEvent(ev)) {
			if (ev->As<NkWindowCloseEvent>() != nullptr) tourne = false;
		}
		if (!tourne) break;

		// la logique avance d'UNE image, puis on dessine ce qu'elle a decide
		monde.Avancer((int)image + 1);

		if (rEcran->BeginFrame()) {
			NkICommandBuffer *cmd = rEcran->GetCmd();
			NkRender3D *r3d = renderer->GetRender3D();
			renderer->FlushGraphRebuilds();
			r3d->ResetFrame();
			if (auto *mc = renderer->GetMaterialCollection()) mc->Upload();

			NkCamera3DData cd;
			cd.position = { 0.30f, 1.50f, 1.90f };
			cd.target = { 0.20f, 0.90f, 0.50f };
			cd.up = { 0.f, 1.f, 0.f };
			cd.fovY = 70.f;
			cd.aspect = (float32)L / (float32)H;
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
			SoumettreLaSalle(r3d, renderer->GetMeshSystem());
			// la main (rouge) et l'objet (vert, rouge s'il tombe)
			Pave(r3d, renderer->GetMeshSystem(), { (float32)monde.paume.x, (float32)monde.paume.y, (float32)monde.paume.z },
				 { 0.08f, 0.08f, 0.08f }, { 0.85f, 0.15f, 0.15f });
			Pave(r3d, renderer->GetMeshSystem(), { (float32)monde.objet.pose.x, (float32)monde.objet.pose.y, (float32)monde.objet.pose.z },
				 { 0.15f, 0.15f, 0.15f }, monde.objet.enChute ? NkVec3f{ 0.95f, 0.55f, 0.10f } : NkVec3f{ 0.15f, 0.75f, 0.30f });

			if (auto *graph = renderer->GetRenderGraph()) graph->Execute(cmd);
			if (auto *r2d = rEcran->GetRender2D()) {
				r2d->DrawImage(cible->GetColorResult(), { 0.f, 0.f, (float32)devInit.width, (float32)devInit.height });
			}
			rEcran->Present();
			rEcran->EndFrame();
		}
		++image;

		const EtatAction &e = monde.saisir.Etat();
		if (journal != nullptr && image <= 60) {
			std::fprintf(journal, "image : %llu, courant : %d, change : %d, actif : %d\n",
						 (unsigned long long)image, e.courant ? 1 : 0, e.change ? 1 : 0, e.actif ? 1 : 0);
			std::fflush(journal);
		}
		if (image == 46) {
			const bool ok = cible->Capture((fautif ? "front.png" : "niveau.png"));
			std::printf("[Sim] capture image %llu -> %s (%s)\n", (unsigned long long)image, (fautif ? "front.png" : "niveau.png"), ok ? "ok" : "echec");
		}
		if (image >= imageDeSortie) tourne = false;
	}
	std::printf("[Sim] %llu images, objet tenu : %d, y de l'objet : %.3f\n", (unsigned long long)image, monde.objet.tenu ? 1 : 0, monde.objet.pose.y);
	if (journal != nullptr) std::fclose(journal);
	NkRenderer::Destroy(renderer);
	NkRenderer::Destroy(rEcran);
	NkDeviceFactory::Destroy(device);
	return 0;
}