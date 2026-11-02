// =============================================================================
//  Exercice 10 : un meilleur modele.
//
//  Sans fenetre, au demarrage : on travaille sur le lacet seul, en degres, sans quaternion. Echantillonne a
//  72 Hz, on predit une periode plus loin :
//    - le modele a VITESSE constante :  lacet + vitesse * periode   (vitesse tiree des deux dernieres poses) ;
//    - le modele a ACCELERATION : on y ajoute  acceleration * periode * periode / 2   (tiree des trois dernieres poses).
//  Quatre enregistrements : un geste lent, le quart de tour en 400 ms, un demi-tour brusque, et un arret brusque
//  (la tete tourne a vitesse constante, puis s'arrete net). Erreur moyenne et plus grande, en degres, pendant le
//  geste et les 200 ms qui suivent.
//
//  Une ligne par enregistrement :
//      rapide (90 deg en 400 ms) : vitesse V deg (max X), acceleration A deg (max Y)
//  MASALLE_DETAIL=1 imprime en plus, pose par pose, ce que chaque modele predit.
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
#include <cstdarg>
#include <cmath>
#include <vector>
#include <algorithm>
#include "NKEvent/NkMouseEvent.h"
#include "NKMath/NkQuat.h"
#include "NKMath/NkAngle.h"
#include "NKMath/NkMat.h"
#include "NKTime/NkChrono.h"

using namespace nkentseu;
using namespace nkentseu::renderer;

static void ConfigureAppData(NkAppData &d) { d.appName = "Modeles"; }
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

namespace {

	const double kPi = 3.14159265358979323846;

	// Une ligne destinee au fichier .txt de l'exercice : elle s'affiche ET s'ajoute a un fichier de sortie.
	// Effacez ce fichier avant la premiere mesure (Remove-Item), il est ecrit A LA SUITE.
	const char *const gFichierSortie = "modeles-sortie.txt";
	void Livrer(const char *format, ...) {
		char tampon[512];
		va_list args;
		va_start(args, format);
		std::vsnprintf(tampon, sizeof(tampon), format, args);
		va_end(args);
		std::fputs(tampon, stdout);
		std::fflush(stdout);
		if (FILE *f = std::fopen(gFichierSortie, "a")) {
			std::fputs(tampon, f);
			std::fclose(f);
		}
	}


	// ---- CALCUL : debut ----
	struct Enregistrement {
		const char *nom;
		bool arretNet;         // vrai : vitesse constante puis arret net ; faux : depart et arrivee doux
		double amplitudeDeg;   // doux : l'amplitude ; arret : la vitesse de rotation en deg/s
		double dureeMs;        // la duree du geste (pour l'arret : la duree de la rotation avant l'arret)
	};

	// Les quatre enregistrements. Le rapide est celui de l'enonce ; les autres sont des choix, modifiables ici.
	const Enregistrement kEnregistrements[4] = {
		{ "lent", false, 30.0, 1200.0 },          // 30 degres en 1,2 s
		{ "rapide", false, 90.0, 400.0 },         // un quart de tour en 400 ms (l'enonce)
		{ "demi-tour", false, 180.0, 250.0 },     // 180 degres en 250 ms
		{ "arret", true, 200.0, 530.0 },          // 200 deg/s pendant 530 ms (106 degres), puis arret net
	};

	// Un quart de tour en 400 ms, depart et arrivee doux (la forme de l'enonce), ici avec amplitude et duree
	// en parametres : amplitude 90 et duree 0,400 redonnent exactement la fonction de l'enonce.
	double Lacet(double t, double amplitude, double duree) {   // t en secondes, lacet en degres
		if (t <= 0.0) return 0.0;
		if (t >= duree) return amplitude;
		return amplitude * 0.5 * (1.0 - std::cos(kPi * t / duree));
	}

	// Le lacet (en degres) de l'enregistrement a l'instant t.
	double LacetDe(const Enregistrement &e, double t) {
		if (e.arretNet) {
			const double duree = e.dureeMs * 1e-3;
			if (t <= 0.0) return 0.0;
			return e.amplitudeDeg * (t < duree ? t : duree);     // vitesse constante, puis plus rien
		}
		return Lacet(t, e.amplitudeDeg, e.dureeMs * 1e-3);
	}

	void Simuler(const Enregistrement &e, bool detail) {
		const double P = 1.0 / 72.0;                              // la periode, en secondes
		const double duree = e.dureeMs * 1e-3;
		double sV = 0.0, sA = 0.0, mV = 0.0, mA = 0.0;
		int n = 0;
		if (detail) std::printf("  %s : t(ms) lacet vitesse(deg/s) accel(deg/s2) pred-vitesse pred-accel vrai erreurV erreurA\n", e.nom);
		for (int k = 0; k * P <= duree + 0.200 + 1e-9; ++k) {     // pendant le geste et les 200 ms qui suivent
			const double t = k * P;
			const double l0 = LacetDe(e, t);                      // la pose d'aujourd'hui
			const double l1 = LacetDe(e, t - P);                  // celle d'une periode avant
			const double l2 = LacetDe(e, t - 2.0 * P);            // celle de deux periodes avant
			const double vitesse = (l0 - l1) / P;                 // tiree des deux dernieres poses, en deg/s
			const double acceleration = (l0 - 2.0 * l1 + l2) / (P * P);   // tiree des trois dernieres : deg/s^2
			const double predV = l0 + vitesse * P;                                  // vitesse constante
			const double predA = l0 + vitesse * P + acceleration * P * P / 2.0;     // avec acceleration
			const double vrai = LacetDe(e, t + P);                // la vraie pose de l'instant d'affichage
			const double eV = std::fabs(predV - vrai), eA = std::fabs(predA - vrai);
			sV += eV;
			sA += eA;
			if (eV > mV) mV = eV;
			if (eA > mA) mA = eA;
			++n;
			if (detail) std::printf("  %7.2f %8.3f %10.2f %12.1f %10.3f %10.3f %9.3f %8.4f %8.4f\n", t * 1e3, l0, vitesse,
									acceleration, predV, predA, vrai, eV, eA);
		}
		if (e.arretNet) {
			Livrer("%s (%.0f deg/s pendant %.0f ms) : vitesse %.4f deg (max %.4f), acceleration %.4f deg (max %.4f)\n",
				   e.nom, e.amplitudeDeg, e.dureeMs, sV / n, mV, sA / n, mA);
		} else {
			Livrer("%s (%.0f deg en %.0f ms) : vitesse %.4f deg (max %.4f), acceleration %.4f deg (max %.4f)\n",
				   e.nom, e.amplitudeDeg, e.dureeMs, sV / n, mV, sA / n, mA);
		}
		std::printf("  (%d poses)\n", n);
	}

	void Calculer() {
		const bool detail = (EnvU64("MASALLE_DETAIL", 0) == 1);
		for (const Enregistrement &e : kEnregistrements) Simuler(e, detail);
	}
	// ---- CALCUL : fin ----


} // namespace

int nkmain(const NkEntryState &state) {
	(void)state;
	// Le calcul se fait au demarrage, avant d'ouvrir la fenetre.
	Calculer();
	// MASALLE_CALCUL_SEUL=1 : on s'arrete apres le calcul, sans ouvrir la fenetre.
	if (EnvU64("MASALLE_CALCUL_SEUL", 0) == 1) {
		return 0;
	}

	NkWindowConfig cfg;
	cfg.title = "Ma salle";
	cfg.width = 1280;
	cfg.height = 720;
	cfg.centered = true;
	cfg.vsync = true;

	NkWindow fenetre(cfg);
	if (!fenetre.IsValid()) {
		std::printf("[MaSalle] la fenetre n'a pas pu s'ouvrir\n");
		return 1;
	}

	NkDeviceInitInfo devInit{};
	devInit.surface = fenetre.GetSurfaceDesc();
	devInit.width = fenetre.GetSize().x;
	devInit.height = fenetre.GetSize().y;

	NkIDevice *device = NkDeviceFactory::CreateAutoDetect(devInit);
	if (device == nullptr) {
		std::printf("[MaSalle] aucune API graphique disponible\n");
		return 1;
	}

	// ── Deux renderers, et c'est voulu ─────────────────────────────────────
	//
	// Le premier POSSÈDE l'image : c'est lui qui appelle BeginFrame, Present et
	// EndFrame, et il ne dessine que du 2D.
	//
	// Le second dessine la salle, mais PAS à l'écran : dans une cible hors
	// écran, une texture qu'on peut relire. On affiche ensuite cette texture
	// dans la fenêtre, et on peut l'enregistrer en PNG.
	//
	// Pourquoi ce détour, alors que dessiner directement à l'écran marcherait ?
	// Deux raisons, et la seconde est celle qui compte pour ce livre :
	//   - une chaîne d'échange ne se relit pas, donc on ne peut pas la capturer,
	//     donc on ne peut pas vérifier une image autrement qu'à l'œil ;
	//   - au chapitre 9, il faudra dessiner DEUX fois, dans deux cibles
	//     différentes. Autant monter la machinerie maintenant, avec une seule.
	//
	// Un renderer par cible, et non une cible qu'on change : SetFinalColorTarget
	// reconstruit le graphe de rendu, et le faire à chaque image est banni.
	NkRendererConfig cfgEcran =
		NkRendererConfig::For2D(devInit.api, devInit.width, devInit.height);
	NkRenderer *rEcran = NkRenderer::Create(device, cfgEcran);
	if (rEcran == nullptr || !rEcran->Initialize()) {
		std::printf("[MaSalle] le renderer d'ecran n'a pas pu s'initialiser\n");
		NkDeviceFactory::Destroy(device);
		return 1;
	}

	NkOffscreenDesc od;
	od.width = devInit.width;
	od.height = devInit.height;
	od.hasDepth = true;
	od.hdr = false;
	od.colorFmt = NkGPUFormat::NK_RGBA8_UNORM;
	od.readable = true;
	od.readback = true; // sans quoi Capture() ne peut rien relire
	od.name = "MaSalle_Cible";

	NkOffscreenTarget *cible = rEcran->CreateOffscreen(od);
	if (cible == nullptr) {
		std::printf("[MaSalle] cible hors ecran refusee\n");
		NkRenderer::Destroy(rEcran);
		NkDeviceFactory::Destroy(device);
		return 1;
	}

	NkRendererConfig cfgSalle =
		NkRendererConfig::ForGame(devInit.api, devInit.width, devInit.height);
	cfgSalle.Disable(NK_SS_RENDER2D);
	cfgSalle.Disable(NK_SS_TEXT);
	cfgSalle.Disable(NK_SS_OVERLAY);
	cfgSalle.Enable(NK_SS_OFFSCREEN);

	// NK_SS_POST_PROCESS porte le tonemap, le bloom, le FXAA et le SSAO.
	// MASALLE_POST=0 le coupe : c'est l'expérience qui dit si un défaut
	// d'image vient de la scène ou d'un effet écran. Voir le journal de bord.
	if (EnvU64("MASALLE_POST", 1) == 0) {
		cfgSalle.Disable(NK_SS_POST_PROCESS);
	}

	NkRenderer *renderer = NkRenderer::Create(device, cfgSalle);
	if (renderer == nullptr || !renderer->Initialize()) {
		std::printf("[MaSalle] le renderer de la salle n'a pas pu s'initialiser\n");
		NkRenderer::Destroy(rEcran);
		NkDeviceFactory::Destroy(device);
		return 1;
	}
	renderer->SetFinalColorTarget(
		rEcran->GetTextures()->GetRHIHandle(cible->GetColorResult()));

	std::printf("[MaSalle] salle de %.1f m sur %.1f m, plafond a %.2f m\n",
				(double)kPiece, (double)kPiece, (double)kPlafond);

	// Sortie automatique, pour que la construction se vérifie sans main
	// humaine : MASALLE_EXIT=<image> arrête proprement.
	const uint64 imageDeSortie = EnvU64("MASALLE_EXIT", 0);
	const uint64 imageDeCapture = EnvU64("MASALLE_SHOT", 0);

	uint64 image = 0;
	bool tourne = true;

	while (tourne && fenetre.IsOpen()) {
		NkEvent *ev = nullptr;
		while (NkEvents().PollEvent(ev)) {
			if (ev->As<NkWindowCloseEvent>() != nullptr) {
				tourne = false;
			}
		}
		if (!tourne) break;

		if (rEcran->BeginFrame()) {
			NkICommandBuffer *cmd = rEcran->GetCmd();

			NkRender3D *r3d = renderer->GetRender3D();

			// Ce que BeginFrame ferait si ce renderer possédait l'image. En
			// mode partagé, on le rejoue à la main. Oublier ResetFrame piétine
			// les tampons uniformes d'une image à l'autre.
			renderer->FlushGraphRebuilds();
			r3d->ResetFrame();
			if (auto *mc = renderer->GetMaterialCollection()) {
				mc->Upload();
			}

			NkCamera3DData cd;
			// À hauteur d'yeux, un mètre en avant du mur du fond, regardant
			// la porte. Une caméra fixe : la tête arrive au chapitre 7.
			cd.position = { 0.30f, kYeux, 1.20f };
			cd.target = { -0.70f, 1.10f, -kDemi };
			cd.up = { 0.f, 1.f, 0.f };
			cd.fovY = 70.f;
			cd.aspect = (float32)devInit.width / (float32)devInit.height;
			cd.nearPlane = 0.05f;
			cd.farPlane = 100.f;

			NkSceneContext sctx;
			sctx.camera = NkCamera3D(cd);
			sctx.time = (float32)image / 60.f;
			// La part de lumière qui vient du ciel (0 = aucune, 1 = toute).
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

			// Le graphe de la salle s'exécute sur le command buffer de l'écran,
			// AVANT son Present : deux passes imbriquées sont interdites.
			if (auto *graph = renderer->GetRenderGraph()) {
				graph->Execute(cmd);
			}

			// La texture, montrée dans la fenêtre. C'est tout ce que fait le
			// renderer d'écran : recopier une image déjà faite.
			if (auto *r2d = rEcran->GetRender2D()) {
				r2d->DrawImage(cible->GetColorResult(),
							   { 0.f, 0.f, (float32)devInit.width,
								 (float32)devInit.height });
			}

			rEcran->Present();
			rEcran->EndFrame();
		}

		++image;

		// La capture, à une image nommée : déterministe, reproductible, et
		// c'est elle qui fait les figures du livre.
		if (imageDeCapture != 0 && image == imageDeCapture) {
			const char *chemin = getenv("MASALLE_SHOT_PATH");
			if (chemin == nullptr || *chemin == '\0') {
				chemin = "masalle.png";
			}
			const bool ok = cible->Capture(chemin);
			std::printf("[MaSalle] capture image %llu -> %s (%s)\n",
						(unsigned long long)image, chemin, ok ? "ok" : "echec");
		}

		if (imageDeSortie != 0 && image >= imageDeSortie) {
			tourne = false;
		}
	}

	std::printf("[MaSalle] %llu images\n", (unsigned long long)image);

	NkRenderer::Destroy(renderer);
	NkRenderer::Destroy(rEcran);
	NkDeviceFactory::Destroy(device);
	return 0;
}
