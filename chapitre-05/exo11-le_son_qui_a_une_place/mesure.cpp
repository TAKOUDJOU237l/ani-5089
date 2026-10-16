#include <utility>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>

#include "NKPlatform/NkPlatformDetect.h"
#include "NKWindow/NKMain.h"
#include "NKAudio/NkAudio.h"
#include <vector>

using namespace nkentseu;

static void ConfigureAppData(NkAppData &d) {
	d.appName = "Mesure";
}
NK_REGISTER_ENTRY_APPDATA_UPDATER(ConfigureAppData)

using namespace nkentseu::audio;

static void Rms(AudioEngine &moteur, std::vector<float> &tampon, double &gauche, double &droite) {
	const int TOTAL = 48000, BLOC = 256;
	double sg = 0.0, sd = 0.0;
	int fait = 0;
	while (fait < TOTAL) {
		const int n = (TOTAL - fait < BLOC) ? (TOTAL - fait) : BLOC;
		moteur.RenderToBuffer(tampon.data(), n, 2);
		for (int i = 0; i < n; ++i) {
			const double g = tampon[2 * i], d = tampon[2 * i + 1];
			sg += g * g;
			sd += d * d;
		}
		fait += n;
	}
	gauche = std::sqrt(sg / TOTAL);
	droite = std::sqrt(sd / TOTAL);
}

int nkmain(const NkEntryState &state) {
	(void)state;

	AudioEngineConfig cfg;
	cfg.backend = AudioBackendType::NULL_OUTPUT;
	cfg.sampleRate = 48000;
	AudioEngine &moteur = AudioEngine::Instance();
	if (!moteur.Initialize(cfg)) {
		std::printf("Init audio KO\n");
		return 1;
	}
	moteur.SetListenerPosition(0.f, 0.f, 0.f);
	moteur.SetListenerOrientation(0.f, 0.f, -1.f, 0.f, 1.f, 0.f);

	AudioSample pluie = AudioGenerator::GenerateNoise(4.f, WaveformType::NOISE_PINK);
	std::vector<float> tampon(256 * 2);

	const char *noms[3] = {"gauche", "devant", "droite"};
	const float positions[3][3] = {{-2.f, 0.f, 0.f}, {0.f, 0.f, -2.f}, {2.f, 0.f, 0.f}};

	FILE *f = std::fopen("mesures.txt", "w");
	if (!f)
		return 2;

	for (int i = 0; i < 3; ++i) {
		for (int positional = 1; positional >= 0; --positional) {
			VoiceParams p;
			p.looping = true;
			p.source3d.position[0] = positions[i][0];
			p.source3d.position[1] = positions[i][1];
			p.source3d.position[2] = positions[i][2];
			p.source3d.positional = (positional == 1);
			AudioHandle h = moteur.Play(pluie, p);

			double g = 0.0, d = 0.0;
			Rms(moteur, tampon, g, d);
			moteur.Stop(h);

			std::fprintf(f, "%s %d : G %.6f D %.6f\n", noms[i], positional, g, d);
			std::printf("%s %d : G %.6f D %.6f\n", noms[i], positional, g, d);
		}
	}
	std::fclose(f);

	moteur.Shutdown();
	AudioLoader::Free(pluie);
	return 0;
}
