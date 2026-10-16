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
	d.appName = "Attenuations";
}
NK_REGISTER_ENTRY_APPDATA_UPDATER(ConfigureAppData)

using namespace nkentseu::audio;

static double RmsDeuxOreilles(AudioEngine &moteur, std::vector<float> &tampon) {
	const int TOTAL = 48000, BLOC = 256;
	double somme = 0.0;
	int fait = 0;
	while (fait < TOTAL) {
		const int n = (TOTAL - fait < BLOC) ? (TOTAL - fait) : BLOC;
		moteur.RenderToBuffer(tampon.data(), n, 2);
		for (int i = 0; i < 2 * n; ++i)
			somme += (double)tampon[i] * tampon[i];
		fait += n;
	}
	return std::sqrt(somme / (2.0 * TOTAL));
}

static double Mesurer(AudioEngine &moteur, const AudioSample &son, std::vector<float> &tampon,
                      AttenuationModel modele, float rolloff, float distance) {
	VoiceParams p;
	p.looping = true;
	p.source3d.positional = true;
	p.source3d.minDistance = 1.f;
	p.source3d.maxDistance = 100.f;
	p.source3d.rolloffFactor = rolloff;
	p.source3d.attenuationModel = modele;
	p.source3d.position[0] = 0.f;
	p.source3d.position[1] = 0.f;
	p.source3d.position[2] = -distance;
	AudioHandle h = moteur.Play(son, p);
	const double rms = RmsDeuxOreilles(moteur, tampon);
	moteur.Stop(h);
	return rms;
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

	AudioSample son = AudioGenerator::GenerateTone(440.f, 2.f, WaveformType::SINE, 48000, 0.1f);
	std::vector<float> tampon(256 * 2);

	const AttenuationModel modeles[4] = {AttenuationModel::NONE, AttenuationModel::INVERSE, AttenuationModel::LINEAR, AttenuationModel::EXPONENTIAL};
	const char *noms[4] = {"NONE", "INVERSE", "LINEAR", "EXPONENTIAL"};
	const float distances[5] = {0.5f, 1.f, 2.f, 4.f, 8.f};
	const float rolloffs[2] = {1.f, 2.f};

	FILE *f = std::fopen("mesures.txt", "w");
	if (!f)
		return 2;

	for (int r = 0; r < 2; ++r) {
		for (int m = 0; m < 4; ++m) {
			for (int d = 0; d < 5; ++d) {
				const double mesure = Mesurer(moteur, son, tampon, modeles[m], rolloffs[r], distances[d]);
				const double reference = Mesurer(moteur, son, tampon, AttenuationModel::NONE, rolloffs[r], distances[d]);
				const double gain = reference > 0.0 ? mesure / reference : 0.0;
				std::fprintf(f, "%s, rolloff %g, %g m : %.3f\n", noms[m], rolloffs[r], distances[d], gain);
				std::printf("%s, rolloff %g, %g m : %.3f\n", noms[m], rolloffs[r], distances[d], gain);
			}
		}
	}
	std::fclose(f);

	moteur.Shutdown();
	AudioLoader::Free(son);
	return 0;
}
