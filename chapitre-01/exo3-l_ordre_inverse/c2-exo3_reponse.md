#include <iostream>
#include <iomanip>

struct Vecteur { double x, y, z; };
struct Quaternion { double x, y, z, w; };
struct Pose { Vecteur position; Quaternion orientation; };

Vecteur tourner(const Quaternion& q, const Vecteur& p)
{
    return {
        (1 - 2 * (q.y * q.y + q.z * q.z)) * p.x
            + 2 * (q.x * q.y - q.z * q.w) * p.y
            + 2 * (q.x * q.z + q.y * q.w) * p.z,
        2 * (q.x * q.y + q.z * q.w) * p.x
            + (1 - 2 * (q.x * q.x + q.z * q.z)) * p.y
            + 2 * (q.y * q.z - q.x * q.w) * p.z,
        2 * (q.x * q.z - q.y * q.w) * p.x
            + 2 * (q.y * q.z + q.x * q.w) * p.y
            + (1 - 2 * (q.x * q.x + q.y * q.y)) * p.z
    };
}


Vecteur rotationPuisTranslation(const Pose& pose, const Vecteur& p)
{
    Vecteur r = tourner(pose.orientation, p);
    return { r.x + pose.position.x, r.y + pose.position.y, r.z + pose.position.z };
}


Vecteur translationPuisRotation(const Pose& pose, const Vecteur& p)
{
    Vecteur s = { p.x + pose.position.x, p.y + pose.position.y, p.z + pose.position.z };
    return tourner(pose.orientation, s);
}

int main()
{
    Pose pose;
    Vecteur point;

    std::cin >> pose.position.x >> pose.position.y >> pose.position.z
             >> pose.orientation.x >> pose.orientation.y
             >> pose.orientation.z >> pose.orientation.w
             >> point.x >> point.y >> point.z;

    Vecteur a = rotationPuisTranslation(pose, point);
    Vecteur b = translationPuisRotation(pose, point);

    std::cout << std::fixed << std::setprecision(4)
              << a.x << ' ' << a.y << ' ' << a.z << '\n'
              << b.x << ' ' << b.y << ' ' << b.z << '\n';

    return 0;
}
## Travaillons avec les donnees suivant 

Pose : position t = (0, 0, 1), rotation de 90° autour de l'axe z , quaternion (x, y, z, w) = (0, 0, 0.7071068, 0.7071068) , Point : p = (1, 0, 0)

Résultats :

- Rotation puis translation : R(p) + t   = (0, 1, 0) + (0, 0, 1) = (0, 1, 1)
- Translation puis rotation : R(p + t)   = R(1, 0, 1)            = (0, 1, 1)

Les deux résultats coïncident.

## Pourquoi

La rotation R est linéaire, donc R(p + t) = R(p) + R(t).

On compare :
- rotation puis translation : R(p) + t
- translation puis rotation : R(p) + R(t)

Elles sont égales si et seulement si R(t) = t.

Une rotation d'angle non nul ne laisse fixes que les vecteurs portés par son axe (et le vecteur nul). Les deux ordres donnent donc le même résultat quand la translation t est parallèle à l'axe de rotation, ou quand t = 0,ou quand la rotation est l'identité. Le point p peut être quelconque.

Ici l'axe est z et t = (0, 0, 1) est sur cet axe, donc R(t) = t.

## Contre-exemple

Avec t = (1, 0, 0), même rotation, même p :
- R(p) + t = (1, 1, 0)
- R(p + t) = R(2, 0, 0) = (0, 2, 0)
Les résultats diffèrent, car R(t) = (0, 1, 0) ≠ t.