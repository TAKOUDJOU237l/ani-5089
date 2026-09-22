## Exercice 6 

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

Quaternion multiplier(const Quaternion& a, const Quaternion& b)
{
    return {
        a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
        a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
        a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w,
        a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z
    };
}

Quaternion depuisAxeAngle(Vecteur axe, double angleRad)
{
    double norme = std::sqrt(axe.x * axe.x + axe.y * axe.y + axe.z * axe.z);
    axe.x /= norme;
    axe.y /= norme;
    axe.z /= norme;

    double demi = angleRad / 2.0;
    double s = std::sin(demi);

    return { axe.x * s, axe.y * s, axe.z * s, std::cos(demi) };
}

Pose composer(const Pose& parent, const Pose& enfant)
{
    Quaternion orientationMonde = multiplier(parent.orientation, enfant.orientation);
    Vecteur positionMonde = appliquerPose(parent, enfant.position);

    return { positionMonde, orientationMonde };
}

void afficherVecteur(const std::string& nom, const Vecteur& v)
{
    std::cout << nom << " : "
              << v.x << " " << v.y << " " << v.z << '\n';
}

int main()
{
    std::cout << std::fixed << std::setprecision(6);

    const double LONGUEUR_BRAS = 0.30;
    const double LONGUEUR_AVANT_BRAS = 0.25;

    Pose coudeLocal;
    coudeLocal.position = { 0.0, -LONGUEUR_BRAS, 0.0 };
    coudeLocal.orientation = { 0.0, 0.0, 0.0, 1.0 };

    Pose mainLocal;
    mainLocal.position = { 0.0, -LONGUEUR_AVANT_BRAS, 0.0 };
    mainLocal.orientation = { 0.0, 0.0, 0.0, 1.0 };

    Pose epauleMonde;
    epauleMonde.position = { 0.0, 0.0, 0.0 };
    epauleMonde.orientation = { 0.0, 0.0, 0.0, 1.0 };

    Pose coudeMonde = composer(epauleMonde, coudeLocal);
    Pose mainMonde = composer(coudeMonde, mainLocal);

    std::cout << "Epaule au repos\n";
    afficherVecteur("Coude", coudeMonde.position);
    afficherVecteur("Main", mainMonde.position);

    epauleMonde.orientation = depuisAxeAngle({ 0.0, 0.0, 1.0 }, M_PI / 2.0);

    coudeMonde = composer(epauleMonde, coudeLocal);
    mainMonde = composer(coudeMonde, mainLocal);

    std::cout << "\nEpaule tournee de 90 degres\n";
    afficherVecteur("Coude", coudeMonde.position);
    afficherVecteur("Main", mainMonde.position);

    coudeLocal.orientation = depuisAxeAngle({ 0.0, 0.0, 1.0 }, M_PI / 4.0);

    coudeMonde = composer(epauleMonde, coudeLocal);
    mainMonde = composer(coudeMonde, mainLocal);

    std::cout << "\nEpaule tournee + coude fleci de 45 degres\n";
    afficherVecteur("Coude", coudeMonde.position);
    afficherVecteur("Main", mainMonde.position);

    return 0;
}