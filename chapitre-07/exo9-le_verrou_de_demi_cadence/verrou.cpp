// =============================================================================
//  Exercice 9 : le verrou de demi-cadence.
//
//  Le moteur : « Verrou demi-cadence, 36 images par seconde pour un Quest a 72 Hz : mesure 39 a 51
//  images par seconde instables. »  Raison : « Un rythme stable a 72 divise par 2 vaut mieux qu'un
//  rythme oscillant plus haut. »   On le verifie sur VOTRE ecran.
//
//  1. La salle est alourdie : elle est soumise MASALLE_COPIES fois, chaque copie decalee de 5 m en x.
//     Augmentez MASALLE_COPIES jusqu'a ce que la moyenne depasse la periode de l'ecran.
//  2. La periode vient de la frequence de l'ecran : MASALLE_HZ (72 par defaut ; mettez la vraie).
//  3. Le verrou : a la fin de chaque image, on attend que DEUX periodes se soient ecoulees depuis son
//     debut. Pour attendre : NkChrono::SleepMilliseconds, sur une minuterie FINE (BeginPreciseTiming),
//     puis une attente active sur l'horloge pour les derniers instants. Voir AttendreJusqua.
//     Le verrou n'a d'effet que si l'image dure entre UNE et DEUX periodes : au-dela de deux periodes,
//     il n'attend jamais. MASALLE_LARGEUR et MASALLE_HAUTEUR (fenetre plus petite) servent a y amener la salle.
//  4. Mesures : 1000 intervalles entre deux DEBUTS d'image sans verrou, puis 1000 avec, la salle
//     alourdie dans les deux cas (les 30 premieres images de chaque phase sont ecartees).
//
//  Trois lignes :  copies de la salle : K
//                  sans verrou : moyenne M ms, ecart type E ms, max X ms
//                  avec verrou : ...
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

static void ConfigureAppData(NkAppData &d) { d.appName = "Verrou"; }
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

	// Exercice 9 : chaque copie de la salle est decalee de 5 metres le long de x.
	float32 gDecalageX = 0.f;

	// Un pavé posé dans la salle. `centre` est le centre géométrique, `taille`
	// les trois dimensions en mètres : on ne manipule jamais autre chose.
	void Pave(NkRender3D *r3d, NkMeshSystem *meshes, const NkVec3f &centreSalle,
			  const NkVec3f &taille, const NkVec3f &teinte, float32 rugosite = 1.f) {
		const NkVec3f centre = { centreSalle.x + gDecalageX, centreSalle.y, centreSalle.z };
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
			dc.transform = NkMat4f::Translate({ gDecalageX, 0.f, 0.f }) * NkMat4f::Scale({ kPiece, 1.f, kPiece });
			dc.aabb = { { gDecalageX - kDemi, 0.f, -kDemi }, { gDecalageX + kDemi, 0.f, kDemi } };
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

	// L'instant present, en nanosecondes, sur une horloge qui ne recule jamais (NKTime/NkChrono.h).
	// On le garde en float64 : un float32 ne conserve que sept chiffres.
	double MaintenantNs() {
		return NkChrono::Now().nanoseconds;
	}

	double EnvF64(const char *nom, double defaut) {
		const char *t = getenv(nom);
		if (t == nullptr || *t == '\0') return defaut;
		return atof(t);
	}

	// Une ligne destinee au fichier .txt de l'exercice : elle s'affiche ET s'ajoute a un fichier de sortie.
	// Effacez ce fichier avant la premiere mesure (Remove-Item), il est ecrit A LA SUITE.
	const char *const gFichierSortie = "verrou-sortie.txt";
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

	// Moyenne, ecart type (racine de la moyenne des carres des ecarts a la moyenne), mediane et maximum.
	struct Stats {
		double moyenne = 0.0, ecartType = 0.0, mediane = 0.0, maximum = 0.0;
	};
	Stats Statistiques(const std::vector<double> &v) {
		Stats s;
		if (v.empty()) return s;
		double somme = 0.0;
		for (double x : v) {
			somme += x;
			if (x > s.maximum) s.maximum = x;
		}
		s.moyenne = somme / (double)v.size();
		double carres = 0.0;
		for (double x : v) carres += (x - s.moyenne) * (x - s.moyenne);
		s.ecartType = std::sqrt(carres / (double)v.size());
		std::vector<double> tri(v);
		std::sort(tri.begin(), tri.end());
		const size_t n = tri.size();
		s.mediane = (n % 2 == 1) ? tri[n / 2] : 0.5 * (tri[n / 2 - 1] + tri[n / 2]);
		return s;
	}


	// Attend, sans trop depasser, l'instant cible (en nanosecondes, sur l'horloge de MaintenantNs).
	// POURQUOI CE DECOUPAGE : sans BeginPreciseTiming(), NkChrono::SleepMilliseconds dort ~15,5 ms quoi
	// qu'on lui demande (commentaire de NKTime/NkChrono.h). Avec la minuterie fine, il dure a peu pres ce
	// qu'on demande, a une ou deux millisecondes pres, parfois un peu plus. On dort donc par pas de 1 ms
	// tant qu'il reste plus de 2 ms, puis on tourne sur l'horloge (attente active) pour les derniers instants :
	// le sommeil ne sert qu'a ne pas bruler le processeur, la precision vient de l'attente active.
	void AttendreJusqua(double cibleNs) {
		while (cibleNs - MaintenantNs() > 2e6) {
			NkChrono::SleepMilliseconds(1);
		}
		while (MaintenantNs() < cibleNs) {
			// attente active : quelques centaines de microsecondes tout au plus
		}
	}


} // namespace

int nkmain(const NkEntryState &state) {
	(void)state;
	NkChrono::BeginPreciseTiming();           // minuterie fine : sinon SleepMilliseconds dort ~15,5 ms quoi qu'on demande

	NkWindowConfig cfg;
	cfg.title = "Ma salle";
	// MASALLE_LARGEUR / MASALLE_HAUTEUR : la taille de la fenetre (1280 x 720 par defaut). Une fenetre plus petite
	// allege le dessin : c'est un reglage pour amener le temps d'image entre une et deux periodes de l'ecran.
	cfg.width = (uint32)EnvU64("MASALLE_LARGEUR", 1280);
	cfg.height = (uint32)EnvU64("MASALLE_HAUTEUR", 720);
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

	const double hz = (EnvF64("MASALLE_HZ", 72.0) > 1.0) ? EnvF64("MASALLE_HZ", 72.0) : 72.0;
	const double periodeNs = 1e9 / hz;
	const int copies = (int)EnvU64("MASALLE_COPIES", 10);
	std::printf("[verrou] cadence %.1f Hz, periode %.3f ms, copies %d (MASALLE_HZ, MASALLE_COPIES)\n", hz, periodeNs * 1e-6, copies);
	int phase = 0;                                   // 0 : sans verrou ; 1 : avec verrou
	uint64 imagesDeLaPhase = 0;
	double debutPrecedent = 0.0, debutImage = 0.0;
	std::vector<double> intervalles;
	intervalles.reserve(1100);
	Stats sansVerrou;
	while (tourne && fenetre.IsOpen()) {
		debutImage = MaintenantNs();                  // le DEBUT de cette image
		if (imagesDeLaPhase > 30) {                    // les 30 premieres images de la phase sont ecartees
			intervalles.push_back((debutImage - debutPrecedent) * 1e-6);
			if (intervalles.size() >= 1000) {
				const Stats s = Statistiques(intervalles);
				if (phase == 0) {
					Livrer("copies de la salle : %d\n", copies);
					Livrer("sans verrou : moyenne %.3f ms, ecart type %.3f ms, max %.3f ms\n", s.moyenne, s.ecartType, s.maximum);
					sansVerrou = s;
					phase = 1;                         // on passe a la phase avec verrou, sur la meme salle
					imagesDeLaPhase = 0;
					intervalles.clear();
				} else {
					Livrer("avec verrou : moyenne %.3f ms, ecart type %.3f ms, max %.3f ms\n", s.moyenne, s.ecartType, s.maximum);
					std::printf("(periode de l'ecran %.3f ms ; sans verrou %.1f images/s ; avec verrou %.1f images/s)\n",
								periodeNs * 1e-6, 1000.0 / sansVerrou.moyenne, 1000.0 / s.moyenne);
					tourne = false;
				}
			}
		}
		debutPrecedent = debutImage;
		++imagesDeLaPhase;
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
			for (int k = 0; k < copies; ++k) {           // la salle alourdie : K copies, decalees de 5 m en x
				gDecalageX = 5.f * (float32)k;
				SoumettreLaSalle(r3d, renderer->GetMeshSystem());
			}
			gDecalageX = 0.f;

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
		if (phase == 1) {
			// Le verrou : attendre que deux periodes se soient ecoulees depuis le DEBUT de cette image.
			AttendreJusqua(debutImage + 2.0 * periodeNs);
		}

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
	NkChrono::EndPreciseTiming();

	NkRenderer::Destroy(renderer);
	NkRenderer::Destroy(rEcran);
	NkDeviceFactory::Destroy(device);
	return 0;
}