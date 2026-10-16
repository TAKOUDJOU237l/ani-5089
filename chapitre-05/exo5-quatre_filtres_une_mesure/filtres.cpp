#include <utility>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>

#include "NKPlatform/NkPlatformDetect.h"
#include "NKWindow/NKMain.h"
#include "NKImage/Core/NkImage.h"
#include "NKTime/NkChrono.h"

using namespace nkentseu;

static void ConfigureAppData(NkAppData &d) {
	d.appName = "Filtres";
}
NK_REGISTER_ENTRY_APPDATA_UPDATER(ConfigureAppData)

int nkmain(const NkEntryState &state) {
	(void)state;

	NkImage photo;
	if (!photo.Load("photo.jpg")) {
		std::printf("Photo introuvable : photo.jpg\n");
		return 1;
	}

	const NkImage source = photo.Resize(2048, 2048);
	std::printf("source : %d x %d\n", source.Width(), source.Height());

	const char *noms[4] = {"NK_NEAREST", "NK_BILINEAR", "NK_BICUBIC", "NK_LANCZOS3"};
	const char *fichiers[4] = {"nearest.png", "bilinear.png", "bicubic.png", "lanczos3.png"};
	const NkResizeFilter filtres[4] = {
		NkResizeFilter::NK_NEAREST,
		NkResizeFilter::NK_BILINEAR,
		NkResizeFilter::NK_BICUBIC,
		NkResizeFilter::NK_LANCZOS3,
	};

	const int REPETITIONS = 20;
	NkImage resultats[4];
	double moyennes[4];

	for (int i = 0; i < 4; ++i) {
		double total = 0.0;
		for (int r = 0; r < REPETITIONS; ++r) {
			NkChrono chrono;
			NkImage reduite = source.Resize(256, 256, filtres[i]);
			total += (double)chrono.Elapsed().ToMicroseconds();
			if (r == REPETITIONS - 1)
				resultats[i] = std::move(reduite);
		}
		moyennes[i] = total / REPETITIONS;
		resultats[i].SavePNG(fichiers[i]);
	}

	FILE *f = std::fopen("mesures.txt", "w");
	if (!f)
		return 2;

	for (int i = 0; i < 4; ++i) {
		long long differences = 0;
		for (int y = 0; y < 256; ++y) {
			for (int x = 0; x < 256; ++x) {
				const auto a = resultats[i].GetPixel(x, y);
				const auto b = resultats[1].GetPixel(x, y);
				if (a.r != b.r || a.g != b.g || a.b != b.b || a.a != b.a)
					++differences;
			}
		}
		std::fprintf(f, "%s : %.1f us, differences : %lld\n", noms[i], moyennes[i], differences);
		std::printf("%s : %.1f us, differences : %lld\n", noms[i], moyennes[i], differences);
	}
	std::fclose(f);
	return 0;
}
