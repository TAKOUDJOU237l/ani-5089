#include <iostream>
#include <iomanip>

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

Vecteur appliquerPose(const Pose& pose, const Vecteur& point)
{
    const double qx = pose.orientation.x;
    const double qy = pose.orientation.y;
    const double qz = pose.orientation.z;
    const double qw = pose.orientation.w;

    
    Vecteur tourne;

    tourne.x =
        (1 - 2 * (qy * qy + qz * qz)) * point.x
        + 2 * (qx * qy - qz * qw) * point.y
        + 2 * (qx * qz + qy * qw) * point.z;

    tourne.y =
        2 * (qx * qy + qz * qw) * point.x
        + (1 - 2 * (qx * qx + qz * qz)) * point.y
        + 2 * (qy * qz - qx * qw) * point.z;

    tourne.z =
        2 * (qx * qz - qy * qw) * point.x
        + 2 * (qy * qz + qx * qw) * point.y
        + (1 - 2 * (qx * qx + qy * qy)) * point.z;

    
    return {
        tourne.x + pose.position.x,
        tourne.y + pose.position.y,
        tourne.z + pose.position.z
    };
}

int main()
{
    Pose pose;
    Vecteur point;

   
    std::cin >> pose.position.x
             >> pose.position.y
             >> pose.position.z;

    
    std::cin >> pose.orientation.x
             >> pose.orientation.y
             >> pose.orientation.z
             >> pose.orientation.w;

    
    std::cin >> point.x
             >> point.y
             >> point.z;

    Vecteur resultat = appliquerPose(pose, point);

    std::cout << std::fixed << std::setprecision(4)
              << resultat.x << '\n'
              << resultat.y << '\n'
              << resultat.z << '\n';

    return 0;
}

## Exemple d excution 

Entré du programme
Une position  0.0 0.0 0.0
Un quaternion 0.0 0.70710678 0.0 0.70710678
Un point  0.0 0.0 -2.0

sortie 

-2.0000 0.0000 -0.0000

