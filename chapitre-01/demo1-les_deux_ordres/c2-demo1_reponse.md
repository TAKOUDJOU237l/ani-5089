## Exercice 1


#include <iostream>
#include <iomanip>
#include <cmath>

struct Vecteur {
    double x, y, z;
};

struct Quaternion {
    double x, y, z, w;
};

struct Pose {
    Vecteur position;
    Quaternion orientation;
};

Vecteur tourner(const Quaternion& q, const Vecteur& v) {
    const double qx = q.x, qy = q.y, qz = q.z, qw = q.w;
    Vecteur r;
    r.x = (1 - 2*(qy*qy + qz*qz)) * v.x
        + 2*(qx*qy - qz*qw) * v.y
        + 2*(qx*qz + qy*qw) * v.z;
    r.y = 2*(qx*qy + qz*qw) * v.x
        + (1 - 2*(qx*qx + qz*qz)) * v.y
        + 2*(qy*qz - qx*qw) * v.z;
    r.z = 2*(qx*qz - qy*qw) * v.x
        + 2*(qy*qz + qx*qw) * v.y
        + (1 - 2*(qx*qx + qy*qy)) * v.z;
    return r;
}


Vecteur appliquerPose(const Pose& pose, const Vecteur& point) {
    Vecteur tourne = tourner(pose.orientation, point);
    return {
        tourne.x + pose.position.x,
        tourne.y + pose.position.y,
        tourne.z + pose.position.z
    };
}


Quaternion rotationY90() {
    double angle = M_PI_2;  // 90 deg
    double half = angle / 2.0;
    return {0.0, std::sin(half), 0.0, std::cos(half)};
}

int main() {
    Pose poseRotation;
    poseRotation.position = {0, 0, 0};
    poseRotation.orientation = rotationY90();

    Pose poseTranslation;
    poseTranslation.position = {1, 0, 0};  
    poseTranslation.orientation = {0, 0, 0, 1};  

    Vecteur pointDepart = {0, 0, 0};

    
    Vecteur apresRotation = appliquerPose(poseRotation, pointDepart);
    Vecteur cas1 = appliquerPose(poseTranslation, apresRotation);

 
    Vecteur apresTranslation = appliquerPose(poseTranslation, pointDepart);
    Vecteur cas2 = appliquerPose(poseRotation, apresTranslation);

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Point de départ : ("
              << pointDepart.x << ", "
              << pointDepart.y << ", "
              << pointDepart.z << ")\n\n";

    std::cout << "Cas 1 - Tourner puis avancer :\n";
    std::cout << "  Position finale : ("
              << cas1.x << ", "
              << cas1.y << ", "
              << cas1.z << ")\n\n";

    std::cout << "Cas 2 - Avancer puis tourner :\n";
    std::cout << "  Position finale : ("
              << cas2.x << ", "
              << cas2.y << ", "
              << cas2.z << ")\n";

    return 0;
}

Point de départ : (0.00, 0.00, 0.00)

Cas 1 Tourner puis avancer :
  Position finale : (1.00, 0.00, 0.00)

Cas 2 Avancer puis tourner :
  Position finale : (0.00, 0.00, -1.00)