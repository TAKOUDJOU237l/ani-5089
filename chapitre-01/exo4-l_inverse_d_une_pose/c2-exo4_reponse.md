#  Exercice 4 : l'inverse d'une pose



## Code

#include <iostream>
#include <iomanip>
#include <cmath>

struct Vecteur
{
    double x;
    double y;
    double z;
};

struct Quaternion
{
    double x;
    double y;
    double z;
    double w;
};

struct Pose
{
    Vecteur position;
    Quaternion orientation;
};

Vecteur tourner(const Quaternion& q, const Vecteur& v)
{
    const double qx = q.x, qy = q.y, qz = q.z, qw = q.w;

    Vecteur r;

    r.x =
        (1 - 2 * (qy * qy + qz * qz)) * v.x
        + 2 * (qx * qy - qz * qw) * v.y
        + 2 * (qx * qz + qy * qw) * v.z;

    r.y =
        2 * (qx * qy + qz * qw) * v.x
        + (1 - 2 * (qx * qx + qz * qz)) * v.y
        + 2 * (qy * qz - qx * qw) * v.z;

    r.z =
        2 * (qx * qz - qy * qw) * v.x
        + 2 * (qy * qz + qx * qw) * v.y
        + (1 - 2 * (qx * qx + qy * qy)) * v.z;

    return r;
}

Vecteur appliquerPose(const Pose& pose, const Vecteur& point)
{
    Vecteur tourne = tourner(pose.orientation, point);

    return {
        tourne.x + pose.position.x,
        tourne.y + pose.position.y,
        tourne.z + pose.position.z
    };
}


Quaternion conjuguer(const Quaternion& q)
{
    return {-q.x, -q.y, -q.z, q.w};
}


Pose Inverser(const Pose& pose)
{
    Quaternion qInv = conjuguer(pose.orientation);

    Vecteur positionOpposee = {
        -pose.position.x,
        -pose.position.y,
        -pose.position.z
    };

    Vecteur positionInv = tourner(qInv, positionOpposee);

    return {positionInv, qInv};
}

int main()
{
    Pose pose;
    Vecteur point;

    std::cin >> pose.position.x >> pose.position.y >> pose.position.z;
    std::cin >> pose.orientation.x >> pose.orientation.y >> pose.orientation.z >> pose.orientation.w;
    std::cin >> point.x >> point.y >> point.z;

    Pose poseInv = Inverser(pose);

    Vecteur resultat = appliquerPose(pose, point);
    Vecteur retour = appliquerPose(poseInv, resultat);

    Vecteur ecart = {
        retour.x - point.x,
        retour.y - point.y,
        retour.z - point.z
    };

    std::cout << std::fixed << std::setprecision(6);

    std::cout << "Point apres pose : "
              << resultat.x << " " << resultat.y << " " << resultat.z << '\n';

    std::cout << "Point apres pose inverse : "
              << retour.x << " " << retour.y << " " << retour.z << '\n';

    std::cout << "Ecart au point de depart : "
              << ecart.x << " " << ecart.y << " " << ecart.z << '\n';

    return 0;
}


## Pourquoi cette construction de Inverser(pose)

Une pose applique rotation puis translation : y = R x + t.

Pour retrouver x à partir de y, il faut inverser cette opération : x = R^-1 (y - t) = R^-1 y - R^-1 t.

Pour un quaternion unitaire, l'inverse est simplement le conjugué (composantes vectorielles opposées, partie réelle inchangée). Donc R^-1 correspond au quaternion conjugué.

En regardant x = R^-1 y + (- R^-1 t), on retrouve la forme d'une pose normale appliquée à y : une rotation par le conjugué, puis une translation égale à moins la position d'origine, elle-même tournée par ce même conjugué. C'est exactement la construction demandée dans l'énoncé.

## Vérification

Premier essai : position (3, -2, 5), rotation de 90 degrés autour de l'axe Y, point de départ (1, 0, 0).
Résultat après pose puis inverse : (1.0000, 0.0000, 0.0000).
Écart avec le point de départ : 0.0000.

Second essai, avec une rotation quelconque non alignée sur un axe : position (-4.3, 7.1, 2.9), rotation de 37 degrés autour de l'axe (0.2673, 0.5345, 0.8018), point de départ (5.5, -3.2, 1.1).
Résultat après pose puis inverse : (5.5000, -3.2000, 1.1000).
Écart avec le point de départ : 0.0000.

Dans les deux cas, appliquer la pose puis son inverse redonne exactement le point de départ, écart nul aux arrondis près, ce qui confirme que Inverser(pose) est correcte.