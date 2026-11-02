// =============================================================================
//  Exercice 7 : la latence injectee, et la borne.
//
//  Sans fenetre, au demarrage : une tete dont les poses arrivent EN RETARD. On rejoue le simulateur du
//  moteur : un tableau circulaire de 64 poses (instant, orientation, vitesse), SampleAt(now) qui rend
//  l'echantillon d'il y a « latence » secondes, et PredictAt qui extrapole depuis cet echantillon, borne a
//  +-100 ms. Les poses suivent le quart de tour en 400 ms commence a t = 1 s, une pose par periode de
//  72 Hz pendant 3 secondes. Les instants sont gardes en secondes (float64).
//
//  1. Pour des latences de 0, 10, 20, 40 et 80 ms : erreur moyenne et plus grande, avec et sans prediction,
//     de t = 1 s a t = 1,9 s. L'erreur est l'angle avec la VRAIE pose de l'instant d'affichage (t + periode).
//     Les moyennes sont aussi ecrites dans courbe.csv, pour tracer courbe.png (tracer_courbe.py).
//  2. Sans borne, latence d'une seconde : l'image dont l'echantillon est le plus proche de t = 1,2 s ;
//     son instant, l'horizon de la prediction, le lacet predit et le vrai. Puis avec la borne.
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

static void ConfigureAppData(NkAppData &d) { d.appName = "Latence"; }
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

	// L'angle, en degres, qui separe deux orientations : (b * a.Conjugate()).Normalized().Angle().Deg().
	double AngleDeg(const NkQuatf &a, const NkQuatf &b) {
		return (double)(b * a.Conjugate()).Normalized().Angle().Deg();
	}

	// Le lacet d'une rotation autour de l'axe vertical : 2 * atan2(q.y, q.w), ramene dans ]-180, 180].
	double LacetDeQuaternion(const NkQuatf &q) {
		double l = 2.0 * std::atan2((double)q.y, (double)q.w) * 180.0 / kPi;
		while (l > 180.0) l -= 360.0;
		while (l <= -180.0) l += 360.0;
		return l;
	}

	// Une ligne destinee au fichier .txt de l'exercice : elle s'affiche ET s'ajoute a un fichier de sortie.
	// Effacez ce fichier avant la premiere mesure (Remove-Item), il est ecrit A LA SUITE.
	const char *const gFichierSortie = "latences-sortie.txt";
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


	// La vitesse angulaire entre deux poses horodatees : celle du module de realite virtuelle du moteur.
	// Le vecteur rendu a pour direction l'axe de la rotation, pour longueur la vitesse en radians par seconde.
	NkVec3f VitesseAngulaire(const NkQuatf &from, const NkQuatf &to, float32 dtSeconds) {
		if (dtSeconds <= 0.f) {
			return NkVec3f(0.f, 0.f, 0.f);
		}
		NkQuatf delta = (to * from.Conjugate()).Normalized();
		// Les deux hemispheres du quaternion codent la meme rotation ; on
		// force le chemin court, sinon un delta minuscule peut se lire
		// comme un tour presque complet dans l'autre sens.
		if (delta.w < 0.f) {
			delta = NkQuatf(-delta.x, -delta.y, -delta.z, -delta.w);
		}
		const float32 angleRad = delta.Angle().Rad();
		if (angleRad <= 1e-6f) {
			return NkVec3f(0.f, 0.f, 0.f);
		}
		return delta.Axis() * (angleRad / dtSeconds);
	}

	// Predire, c'est faire tourner l'orientation a cette vitesse pendant le temps qui reste avant l'affichage.
	NkQuatf Extrapoler(const NkQuatf &orientation, const NkVec3f &vitesse, float32 dtSeconds) {
		const float32 speedRad = vitesse.Len();
		if (speedRad > 1e-6f) {
			const NkVec3f axis = vitesse * (1.f / speedRad);
			// Delta en espace de BASE : il se compose a GAUCHE de
			// l'orientation courante (meme convention que la mesure).
			const NkQuatf delta(NkAngle::FromRad(speedRad * dtSeconds), axis);
			return (delta * orientation).Normalized();
		}
		return orientation;
	}

	// ---- CALCUL : debut ----
	// Un quart de tour en 400 ms, depart et arrivee doux (la forme de l'enonce), ici avec amplitude et duree
	// en parametres : amplitude 90 et duree 0,400 redonnent exactement la fonction de l'enonce.
	double Lacet(double t, double amplitude, double duree) {   // t en secondes, lacet en degres
		if (t <= 0.0) return 0.0;
		if (t >= duree) return amplitude;
		return amplitude * 0.5 * (1.0 - std::cos(kPi * t / duree));
	}

	// Ce que le simulateur garde de chaque pose : son instant, son orientation, sa vitesse angulaire.
	struct Echantillon {
		double temps;           // secondes
		NkQuatf orientation;
		NkVec3f vitesse;        // rad/s, autour d'un axe
	};

	const uint32 kMaxEchantillons = 64;    // le tableau circulaire du simulateur

	struct Historique {
		Echantillon echantillons[kMaxEchantillons];
		uint32 tete = 0;                   // l'indice du plus recent
		uint32 compte = 0;                 // le nombre d'echantillons valides
		double latenceSecondes = 0.0;

		void Ajouter(const Echantillon &e) {
			tete = (compte == 0) ? 0 : (tete + 1) % kMaxEchantillons;
			echantillons[tete] = e;
			if (compte < kMaxEchantillons) ++compte;
		}

		// Le meme code que NkXrSimulatorBackend::SampleAt, en secondes.
		const Echantillon &EchantillonA(double maintenant) const {
			// Latence nulle : l'echantillon le plus recent, point.
			if (latenceSecondes <= 0.0 || compte <= 1u) {
				return echantillons[tete];
			}
			// Latence simulee : on rend l'echantillon d'il y a « latence » secondes.
			const double cible = maintenant - latenceSecondes;
			uint32 index = tete;
			for (uint32 i = 0u; i < compte; ++i) {
				if (echantillons[index].temps <= cible) {
					return echantillons[index];
				}
				index = (index + kMaxEchantillons - 1u) % kMaxEchantillons;
			}
			// Plus vieux disponible : l'historique ne remonte pas si loin.
			return echantillons[(tete + kMaxEchantillons - (compte - 1u)) % kMaxEchantillons];
		}
	};

	// Pose predite a l'instant demande ; avec borne, la fenetre d'extrapolation est limitee a +-100 ms.
	NkQuatf Predire(const Echantillon &e, double quand, bool borne) {
		float dt = (float)(quand - e.temps);
		if (borne) {
			dt = std::max(-0.1f, std::min(0.1f, dt));        // NkClamp(dt, -0.1f, 0.1f)
		}
		return Extrapoler(e.orientation, e.vitesse, dt);
	}

	NkQuatf PoseVraie(double t) {      // la vraie pose a l'instant t : le quart de tour commence a t = 1 s
		return NkQuatf::RotateY(NkAngle((float)Lacet(t - 1.0, 90.0, 0.400)));
	}

	// Rejoue trois secondes de poses (une par periode de 72 Hz) ; a chaque nouvelle pose, appelle f(maintenant, historique).
	template <class F>
	void Rejouer(double latenceSecondes, F f) {
		const double periode = 1.0 / 72.0;
		Historique h;
		h.latenceSecondes = latenceSecondes;
		NkQuatf precedente;
		for (int k = 0; k * periode <= 3.0 + 1e-9; ++k) {
			const double t = k * periode;
			const NkQuatf q = PoseVraie(t);
			const NkVec3f v = (k == 0) ? NkVec3f(0.f, 0.f, 0.f) : VitesseAngulaire(precedente, q, (float)periode);
			Echantillon e;
			e.temps = t;
			e.orientation = q;
			e.vitesse = v;
			h.Ajouter(e);
			f(t, h);
			precedente = q;
		}
	}

	struct Erreurs {
		double moyenne = 0.0, maximum = 0.0;
		int n = 0;
	};

	// L'erreur de la prediction (ou de la pose prise telle quelle) de t = 1 s a t = 1,9 s.
	Erreurs Mesurer(double latenceSecondes, bool avecPrediction) {
		const double periode = 1.0 / 72.0;
		Erreurs r;
		double somme = 0.0;
		Rejouer(latenceSecondes, [&](double maintenant, const Historique &h) {
			if (maintenant < 1.0 - 1e-9 || maintenant > 1.9 + 1e-9) return;
			const Echantillon &e = h.EchantillonA(maintenant);
			const double quand = maintenant + periode;          // l'instant ou l'image sera vue
			// On compare a la vraie pose de l'instant d'affichage, pas a celle de l'echantillon.
			const NkQuatf montree = avecPrediction ? Predire(e, quand, true) : e.orientation;
			const double erreur = AngleDeg(montree, PoseVraie(quand));
			somme += erreur;
			if (erreur > r.maximum) r.maximum = erreur;
			++r.n;
		});
		r.moyenne = (r.n > 0) ? somme / r.n : 0.0;
		return r;
	}

	// La ligne de la borne, pour l'image dont l'echantillon est le plus proche de t = 1,2 s.
	void LigneDeLaBorne(double maintenantChoisi, bool borne) {
		const double periode = 1.0 / 72.0;
		Rejouer(1.0, [&](double maintenant, const Historique &h) {
			if (std::fabs(maintenant - maintenantChoisi) > 1e-9) return;
			const Echantillon &e = h.EchantillonA(maintenant);
			const double quand = maintenant + periode;
			const NkQuatf predite = Predire(e, quand, borne);
			Livrer("%s : echantillon a %.4f s, horizon %.1f ms, predit %.2f deg, vrai %.2f deg\n",
				   borne ? "avec borne" : "sans borne", e.temps, (quand - e.temps) * 1e3,
				   LacetDeQuaternion(predite), LacetDeQuaternion(PoseVraie(quand)));
		});
	}

	void Calculer() {
		const double latencesMs[5] = { 0.0, 10.0, 20.0, 40.0, 80.0 };
		FILE *csv = std::fopen("courbe.csv", "w");
		if (csv) std::fprintf(csv, "latence_ms;erreur_avec_deg;erreur_sans_deg\n");
		for (double ms : latencesMs) {
			const Erreurs avec = Mesurer(ms * 1e-3, true);
			const Erreurs sans = Mesurer(ms * 1e-3, false);
			Livrer("latence %.0f ms : avec %.3f deg (max %.3f), sans %.3f deg (max %.3f)\n", ms,
				   avec.moyenne, avec.maximum, sans.moyenne, sans.maximum);
			if (csv) std::fprintf(csv, "%.0f;%.6f;%.6f\n", ms, avec.moyenne, sans.moyenne);
		}
		if (csv) std::fclose(csv);

		// Une latence d'UNE SECONDE : on cherche l'image dont l'echantillon est le plus proche du milieu du geste (1,2 s).
		double choisi = 0.0, meilleurEcart = 1e9;
		Rejouer(1.0, [&](double maintenant, const Historique &h) {
			const double ecart = std::fabs(h.EchantillonA(maintenant).temps - 1.2);
			if (ecart < meilleurEcart) {
				meilleurEcart = ecart;
				choisi = maintenant;
			}
		});
		LigneDeLaBorne(choisi, false);     // sans borne
		LigneDeLaBorne(choisi, true);      // avec la borne remise
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
