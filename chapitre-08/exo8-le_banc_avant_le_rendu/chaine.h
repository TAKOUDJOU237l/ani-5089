#pragma once
#include <cstdint>

// Chaine d'echange : un anneau d'images, un drapeau d'emprunt.
// Les poignees sont des entiers 64 bits sans type : la chaine ne sait
// pas ce qu'elles designent (texture Vulkan, pointeur D3D, entier GL...).
class ChaineEchange {
public:
    static constexpr uint32_t kMaxImages = 4;   // tableau fixe, zero allocation

    // Cree la chaine. Refuse (false) zero image ou plus que le maximum.
    bool Creer(uint32_t nombreImages) {
        if (nombreImages == 0u || nombreImages > kMaxImages) {
            return false;
        }
        mCount = nombreImages;
        mNext = 0u;
        mAcquired = false;
        for (uint32_t i = 0u; i < kMaxImages; ++i) {
            mHandles[i] = 0u;
        }
        return true;
    }

    // Temps 1 : emprunter. Refuse le double emprunt.
    bool AcquireImage(uint32_t &index) {
        if (mCount == 0u || mAcquired) {
            return false;
        }
        mAcquired = true;
        index = mNext;
        return true;
    }

    // Temps 3 : rendre. Refuse si rien n'est emprunte, puis avance l'anneau.
    bool ReleaseImage() {
        if (!mAcquired) {
            return false;
        }
        mAcquired = false;
        mNext = (mNext + 1u) % mCount;
        return true;
    }

    // L'image a soumettre : la derniere rendue, donc un cran en arriere.
    uint32_t GetLastReleasedIndex() const {
        return (mNext + mCount - 1u) % mCount;
    }

    bool SetHandle(uint32_t index, uint64_t handle) {
        if (index >= mCount) {
            return false;
        }
        mHandles[index] = handle;
        return true;
    }

    uint64_t GetHandle(uint32_t index) const {
        return index < mCount ? mHandles[index] : 0u;
    }

    uint32_t Count() const { return mCount; }

private:
    uint64_t mHandles[kMaxImages] = {};
    uint32_t mCount = 0u;
    uint32_t mNext = 0u;
    bool mAcquired = false;
};
