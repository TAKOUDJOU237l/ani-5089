# Exerice 


La fonction est la suivante


#include <cmath>

struct Vecteur {
    double x ;
    double y;
    double  z;
};

struct Quaternion {
    double x ;
    double y ;
    double z ;
    double w;
};

struct Pose {
    Vecteur position;
    Quaternion orientation;
};

struct PoseEtVitesses {
    Pose pose;
    Vecteur vitesseLineaire;      
    Vecteur vitesseAngulaire;     
};


double norme(const Vecteur& v) {
    return std::sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}


Vecteur normaliser(const Vecteur& v) {
    double n = norme(v);
    if (n < 1e-12) return {0, 0, 0};
    return {v.x/n, v.y/n, v.z/n};
}


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


Quaternion quatMult(const Quaternion& q1, const Quaternion& q2) {
    Quaternion r;
    r.x = q1.w*q2.x + q1.x*q2.w + q1.y*q2.z - q1.z*q2.y;
    r.y = q1.w*q2.y - q1.x*q2.z + q1.y*q2.w + q1.z*q2.x;
    r.z = q1.w*q2.z + q1.x*q2.y - q1.y*q2.x + q1.z*q2.w;
    r.w = q1.w*q2.w - q1.x*q2.x - q1.y*q2.y - q1.z*q2.z;
    return r;
}


Quaternion quatNormalize(const Quaternion& q) {
    double n = std::sqrt(q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w);
    if (n < 1e-12) return q;
    return {q.x/n, q.y/n, q.z/n, q.w/n};
}


Pose extrapoler(const Pose& pose,
                const Vecteur& velLin,
                const Vecteur& velAng,
                double dt)
{
    
    Pose resultat;
    resultat.position = {
        pose.position.x + velLin.x * dt,
        pose.position.y + velLin.y * dt,
        pose.position.z + velLin.z * dt
    };

   
    double omega = norme(velAng);
    
    if (omega < 1e-12) {
        
        resultat.orientation = pose.orientation;
    } else {
        double angle = omega * dt;
        Vecteur axe = normaliser(velAng);
        
       
        double halfAngle = angle / 2.0;
        double sinHalf = std::sin(halfAngle);
        Quaternion deltaQ = {
            axe.x * sinHalf,
            axe.y * sinHalf,
            axe.z * sinHalf,
            std::cos(halfAngle)
        };
        
        
        resultat.orientation = quatNormalize(quatMult(pose.orientation, deltaQ));
    }
    
    return resultat;
}


le programme

#include <iostream>
#include <iomanip>



int main() {
    Pose pose;
    Vecteur velLin, velAng;
    double dt;

    
    std::cin >> pose.position.x >> pose.position.y >> pose.position.z;
   
    std::cin >> pose.orientation.x >> pose.orientation.y 
             >> pose.orientation.z >> pose.orientation.w;
    
    std::cin >> velLin.x >> velLin.y >> velLin.z;
    std::cin >> velAng.x >> velAng.y >> velAng.z;

    std::cin >> dt;

    Pose extrapolee = extrapoler(pose, velLin, velAng, dt);

    std::cout << std::fixed << std::setprecision(4);

    std::cout << "Position extrapolée: "
              << extrapolee.position.x << " "
              << extrapolee.position.y << " "
              << extrapolee.position.z << "\n";

    std::cout << "Orientation extrapolée: "
              << extrapolee.orientation.x << " "
              << extrapolee.orientation.y << " "
              << extrapolee.orientation.z << " "
              << extrapolee.orientation.w << "\n";

    return 0;
}