// Deux chaines d'echange, une par oeil, qui survivent a un redimensionnement.
// ATTENTION : version SANS fenetre ni renderer (comptabilite pure, comme le
// banc de l'exercice 8). Le "redimensionnement" est simule a l'image 15 ;
// les textures sont representees par des poignees 64 bits factices.
#include <cstdint>
#include <cstdio>
#include <cstdlib>

class ChaineEchange {
public:
    static constexpr uint32_t kMaxImages = 4;
    bool Creer(uint32_t n) {
        if (n == 0u || n > kMaxImages) return false;
        mCount = n; mNext = 0u; mAcquired = false;
        for (uint32_t i = 0u; i < kMaxImages; ++i) mHandles[i] = 0u;
        return true;
    }
    void Detruire() { mCount = 0u; mNext = 0u; mAcquired = false; }
    bool AcquireImage(uint32_t &index) {
        if (mCount == 0u || mAcquired) return false;
        mAcquired = true; index = mNext; return true;
    }
    bool ReleaseImage() {
        if (!mAcquired) return false;
        mAcquired = false; mNext = (mNext + 1u) % mCount; return true;
    }
    uint32_t GetLastReleasedIndex() const { return (mNext + mCount - 1u) % mCount; }
    bool SetHandle(uint32_t i, uint64_t h) { if (i >= mCount) return false; mHandles[i] = h; return true; }
    uint64_t GetHandle(uint32_t i) const { return i < mCount ? mHandles[i] : 0u; }
    uint32_t Count() const { return mCount; }
private:
    uint64_t mHandles[kMaxImages] = {};
    uint32_t mCount = 0u, mNext = 0u;
    bool mAcquired = false;
};

// Une "texture" factice par image : un nombre sans signification.
static void EnregistrerImages(ChaineEchange &c, uint64_t base) {
    for (uint32_t i = 0u; i < c.Count(); ++i) c.SetHandle(i, base + i);
}

// Les quatre temps pour un oeil. Rend l'index soumis, ou -1 si faute.
static int UneImage(ChaineEchange &c) {
    uint32_t index = 0u;
    if (!c.AcquireImage(index)) return -1;     // 1. emprunter
    /* 2. dessiner : ici, rien (pas de renderer) */
    if (!c.ReleaseImage()) return -1;          // 3. rendre
    const uint32_t soumise = c.GetLastReleasedIndex();
    if (c.GetHandle(soumise) == 0u) return -1; // 4. soumettre : poignee perdue ?
    return (int)soumise;
}

int main() {
    ChaineEchange gauche, droit;
    FILE *trace = std::fopen("trace.txt", "w");
    FILE *resize = std::fopen("resize.txt", "w");
    if (!trace || !resize) return 1;

    gauche.Creer(3u); droit.Creer(3u);
    EnregistrerImages(gauche, 1000u);   // poignees distinctes par oeil
    EnregistrerImages(droit, 2000u);

    for (int image = 1; image <= 30; ++image) {
        if (image == 16) {   // la fenetre change de taille entre 15 et 16
            gauche.Detruire(); std::fprintf(resize, "detruit la chaine gauche\n");
            droit.Detruire();  std::fprintf(resize, "detruit la chaine droite\n");
            gauche.Creer(3u);  std::fprintf(resize, "recree la chaine gauche\n");
            droit.Creer(3u);   std::fprintf(resize, "recree la chaine droite\n");
            EnregistrerImages(gauche, 3000u); std::fprintf(resize, "reenregistre les images de la chaine gauche\n");
            EnregistrerImages(droit, 4000u);  std::fprintf(resize, "reenregistre les images de la chaine droite\n");
        }
        const int g = UneImage(gauche);
        const int d = UneImage(droit);
        if (g < 0 || d < 0) { std::fprintf(stderr, "faute a l'image %d\n", image); return 1; }
        std::fprintf(trace, "image : %d, gauche : %d, droit : %d\n", image, g, d);
    }
    std::fclose(trace);
    std::fclose(resize);
    return 0;
}
