## Exercie 5

#include <iostream>
#include <iomanip>

struct Vecteur { double x, y, z; };
struct Quaternion { double x, y, z, w; };
struct Pose { Vecteur position; Quaternion orientation; };

Vecteur tourner(const Quaternion& q, const Vecteur& v)
{
    const double qx = q.x, qy = q.y, qz = q.z, qw = q.w;
    Vecteur r;

    r.x = (1 - 2 * (qy * qy + qz * qz)) * v.x
        + 2 * (qx * qy - qz * qw) * v.y
        + 2 * (qx * qz + qy * qw) * v.z;

    r.y = 2 * (qx * qy + qz * qw) * v.x
        + (1 - 2 * (qx * qx + qz * qz)) * v.y
        + 2 * (qy * qz - qx * qw) * v.z;

    r.z = 2 * (qx * qz - qy * qw) * v.x
        + 2 * (qy * qz + qx * qw) * v.y
        + (1 - 2 * (qx * qx + qy * qy)) * v.z;

    return r;
}

Vecteur appliquerPose(const Pose& pose, const Vecteur& point)
{
    Vecteur t = tourner(pose.orientation, point);
    return { t.x + pose.position.x, t.y + pose.position.y, t.z + pose.position.z };
}

Quaternion multiplierQuaternion(const Quaternion& q2, const Quaternion& q1)
{
    return {
        q2.w * q1.x + q2.x * q1.w + q2.y * q1.z - q2.z * q1.y,
        q2.w * q1.y - q2.x * q1.z + q2.y * q1.w + q2.z * q1.x,
        q2.w * q1.z + q2.x * q1.y - q2.y * q1.x + q2.z * q1.w,
        q2.w * q1.w - q2.x * q1.x - q2.y * q1.y - q2.z * q1.z
    };
}

Pose Composer(const Pose& pose2, const Pose& pose1)
{
    Quaternion qComp = multiplierQuaternion(pose2.orientation, pose1.orientation);

    Vecteur posComp = tourner(pose2.orientation, pose1.position);
    posComp.x += pose2.position.x;
    posComp.y += pose2.position.y;
    posComp.z += pose2.position.z;

    return { posComp, qComp };
}

int main()
{
    Pose pose1, pose2;
    Vecteur point;

    std::cin >> pose1.position.x >> pose1.position.y >> pose1.position.z;
    std::cin >> pose1.orientation.x >> pose1.orientation.y >> pose1.orientation.z >> pose1.orientation.w;

    std::cin >> pose2.position.x >> pose2.position.y >> pose2.position.z;
    std::cin >> pose2.orientation.x >> pose2.orientation.y >> pose2.orientation.z >> pose2.orientation.w;

    std::cin >> point.x >> point.y >> point.z;

    Vecteur intermediaire = appliquerPose(pose1, point);
    Vecteur resultatSuccessif = appliquerPose(pose2, intermediaire);

    Pose poseComposee = Composer(pose2, pose1);
    Vecteur resultatCompose = appliquerPose(poseComposee, point);

    Vecteur ecart = {
        resultatCompose.x - resultatSuccessif.x,
        resultatCompose.y - resultatSuccessif.y,
        resultatCompose.z - resultatSuccessif.z
    };

    std::cout << std::fixed << std::setprecision(6);

    std::cout << "Applications successives (pose1 puis pose2) : "
              << resultatSuccessif.x << " " << resultatSuccessif.y << " " << resultatSuccessif.z << '\n';

    std::cout << "Composition puis application : "
              << resultatCompose.x << " " << resultatCompose.y << " " << resultatCompose.z << '\n';

    std::cout << "Ecart entre les deux : "
              << ecart.x << " " << ecart.y << " " << ecart.z << '\n';

    return 0;
}

## Verification 

Prenons un exemple concret :

* pose1 : position (1, 0, 0), orientation (0, 0, 0, 1) avec aucune rotation.
* pose2 : position (0, 2, 0), orientation (0, 0, 0.7071, 0.7071) , une rotation de 90° autour de  l'axe z.
* point : (1, 1, 1).

On calcule le résultat de deux façons différentes :

Applications successives : on applique d'abord pose1 au point, puis pose2 au résultat obtenu.
Composition : on calcule une seule fois la pose composée Composer(pose2, pose1), puis on l'applique directement au point de départ.
Les deux méthodes donnent le même point final, à l'affichage près, et l'écart entre les deux résultats est (0.000000, 0.000000, 0.000000). Les petites différences qui pourraient exister en réalité (de l'ordre de 1e-15) viennent uniquement des arrondis de calcul en virgule flottante, et disparaissent à la précision affichée.
Cela confirme la formule : composer deux poses puis appliquer le résultat au point de départ équivaut exactement à appliquer les deux poses l'une après l'autre.
