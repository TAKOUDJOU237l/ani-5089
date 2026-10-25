// deux-yeux.cpp  --  EXERCICE 9 : PARTIE A COMPLETER AVEC etape5_salle.cpp
//
// Ce fichier contient, verifie contre les en-tetes du kit, tout ce qui ne depend pas de la maniere dont VOTRE
// etape5_salle.cpp cree ses deux cibles et ses deux renderers :
//   * la comparaison pixel a pixel de deux images (ecart d'un pixel = plus grande difference sur ses trois canaux),
//   * l'ecriture de l'image de difference (quatre fois l'ecart, borne a 255 : le noir veut dire "pareil"),
//   * l'ecriture des deux lignes de difference.txt.
// Il manque la partie "dessiner la salle deux fois, une par oeil" : elle demande la creation des cibles hors ecran et
// des deux renderers EXACTEMENT comme le fait etape5_salle.cpp, que je n'ai pas. Voir le bloc  >>> A COMPLETER <<<  plus bas.
//
// Lancement de la comparaison seule, quand les captures existent deja :
//   MASALLE_COMPARER=1  compare oeil-gauche.png / oeil-droit.png   -> difference.png
//                        et oubli-gauche.png / oubli-droit.png      -> difference-vue-oubliee.png
//                        puis ecrit difference.txt
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <string>

#include "NKPlatform/NkPlatformDetect.h"
#include "NKWindow/NKMain.h"
#include "NKImage/Core/NkImage.h"
#include "NKMath/NkColor.h"
#include "NKLogger/NkLog.h"

using namespace nkentseu;

static void ConfigureAppData(NkAppData &d) {
	d.appName = "DeuxYeux";
}
NK_REGISTER_ENTRY_APPDATA_UPDATER(ConfigureAppData)

struct Difference {
	long long pixelsDifferents = 0; // pixels dont l'ecart depasse 1
	long long total = 0;            // nombre de pixels compares
	int ecartMax = 0;
	bool ok = false;
};

static int Abs(int v) {
	return v < 0 ? -v : v;
}

static int Max3(int a, int b, int c) {
	int m = a > b ? a : b;
	return m > c ? m : c;
}

// Compare deux fichiers image pixel par pixel et ecrit l'image de la difference.
static Difference Comparer(const char *fichierA, const char *fichierB, const char *sortiePng) {
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
			const int ecart = Max3(Abs((int)pa.r - (int)pb.r), Abs((int)pa.g - (int)pb.g), Abs((int)pa.b - (int)pb.b));
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

int nkmain(const NkEntryState &state) {
	(void)state;

	const char *comparer = std::getenv("MASALLE_COMPARER");
	if (comparer && comparer[0] == '1') {
		const Difference deux = Comparer("oeil-gauche.png", "oeil-droit.png", "difference.png");
		const Difference oubli = Comparer("oubli-gauche.png", "oubli-droit.png", "difference-vue-oubliee.png");
		FILE *f = std::fopen("difference.txt", "w");
		if (deux.ok) {
			std::printf("deux yeux : %lld pixels sur %lld, ecart max %d\n", deux.pixelsDifferents, deux.total, deux.ecartMax);
			if (f)
				std::fprintf(f, "deux yeux : %lld pixels sur %lld, ecart max %d\n", deux.pixelsDifferents, deux.total, deux.ecartMax);
		}
		if (oubli.ok) {
			std::printf("vue oubliee : %lld pixels sur %lld, ecart max %d\n", oubli.pixelsDifferents, oubli.total, oubli.ecartMax);
			if (f)
				std::fprintf(f, "vue oubliee : %lld pixels sur %lld, ecart max %d\n", oubli.pixelsDifferents, oubli.total, oubli.ecartMax);
		}
		if (f)
			std::fclose(f);
		return (deux.ok && oubli.ok) ? 0 : 1;
	}

	// >>> A COMPLETER <<<  (avec votre etape5_salle.cpp)
	//  1. deux cibles hors ecran de 640 x 720, creees comme la salle cree les siennes (CreateOffscreen) ;
	//  2. deux renderers de salle, un par oeil, chacun avec SetFinalColorTarget(sa cible) puis
	//     SetRenderSizeOverride(640, 720) : la taille de sa cible ;
	//  3. la tete en (0,30 ; 1,70 ; 1,20), ecart de 63 mm, decalage le long de la droite de la tete (tete droite, regard -Z) :
	//       oeil gauche = tete - (1, 0, 0) x 0,0315 ; oeil droit = tete + (1, 0, 0) x 0,0315 ;
	//  4. pour chaque oeil : FlushGraphRebuilds, ResetFrame, Upload, meme salle soumise, graphe execute sur le tampon de commandes ;
	//  5. r2d->DrawImage(cible, {x, 0, 640, 720}) avec x = 0 puis 640 ;
	//  6. a l'image 30 : cibleGauche->Capture("oeil-gauche.png") puis cibleDroite->Capture("oeil-droit.png") ;
	//  7. refaire en "oubliant de decaler" (les deux cameras a la place de la tete) : captures oubli-gauche.png et oubli-droit.png ;
	//  8. apres la boucle : relancer ce programme avec MASALLE_COMPARER=1, ou appeler Comparer() ci-dessus.
	std::printf("deux-yeux : la partie rendu est a completer avec etape5_salle.cpp (voir le commentaire)\n");
	return 3;
}
