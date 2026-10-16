#include <utility>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>

#include "NKPlatform/NkPlatformDetect.h"
#include "NKWindow/NKMain.h"
#include "NKImage/Core/NkImage.h"

using namespace nkentseu;

static void ConfigureAppData(NkAppData &d) {
	d.appName = "Formats";
}
NK_REGISTER_ENTRY_APPDATA_UPDATER(ConfigureAppData)

static long long TailleDisque(const char *chemin) {
	std::error_code ec;
	const auto t = std::filesystem::file_size(chemin, ec);
	return ec ? -1 : (long long)t;
}

int nkmain(const NkEntryState &state) {
	(void)state;

	const char *SOURCE = "photo.jpg";
	NkImage photo;
	if (!photo.Load(SOURCE)) {
		std::printf("Photo introuvable : %s\n", SOURCE);
		return 1;
	}
	std::printf("photo : %d x %d\n", photo.Width(), photo.Height());

	struct Format {
		const char *nom;
		const char *fichier;
	};
	const Format formats[5] = {
		{"png", "sortie.png"},
		{"jpeg", "sortie.jpg"},
		{"bmp", "sortie.bmp"},
		{"tga", "sortie.tga"},
		{"qoi", "sortie.qoi"},
	};

	photo.SavePNG(formats[0].fichier);
	photo.SaveJPEG(formats[1].fichier);
	photo.SaveBMP(formats[2].fichier);
	photo.SaveTGA(formats[3].fichier);
	photo.SaveQOI(formats[4].fichier);

	FILE *f = std::fopen("formats.txt", "w");
	if (!f)
		return 2;

	for (int i = 0; i < 5; ++i) {
		const long long octets = TailleDisque(formats[i].fichier);
		NkImage relue;
		const bool lue = relue.Load(formats[i].fichier) && relue.IsValid();
		if (lue) {
			std::fprintf(f, "%s : %lld octets, %d x %d\n", formats[i].nom, octets, relue.Width(), relue.Height());
			const bool memes = relue.Width() == photo.Width() && relue.Height() == photo.Height();
			std::printf("%s : relue, memes dimensions que la photo : %s\n", formats[i].nom, memes ? "oui" : "non");
		} else {
			std::fprintf(f, "%s : %lld octets, illisible\n", formats[i].nom, octets);
			std::printf("%s : ne se relit pas\n", formats[i].nom);
		}
	}
	std::fclose(f);
	std::printf("formats.txt ecrit\n");
	return 0;
}
