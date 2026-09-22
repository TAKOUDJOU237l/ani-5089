# Exercice 9

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


Quaternion quatMult(const Quaternion& q1, const Quaternion& q2) {
    return {
        q1.w*q2.x + q1.x*q2.w + q1.y*q2.z - q1.z*q2.y,
        q1.w*q2.y - q1.x*q2.z + q1.y*q2.w + q1.z*q2.x,
        q1.w*q2.z + q1.x*q2.y - q1.y*q2.x + q1.z*q2.w,
        q1.w*q2.w - q1.x*q2.x - q1.y*q2.y - q1.z*q2.z
    };
}


Quaternion quatConj(const Quaternion& q) {
    return {-q.x, -q.y, -q.z, q.w};
}


Vecteur vitesseAngulaireMoyenne(const Quaternion& q0,
                                 const Quaternion& q1,
                                 double dt,
                                 bool shortPath = true)
{
    
    Quaternion dq = quatMult(q1, quatConj(q0));
    
    
    if (shortPath && dq.w < 0) {
        dq.x = -dq.x; dq.y = -dq.y; dq.z = -dq.z; dq.w = -dq.w;
    }
    
    double x = dq.x, y = dq.y, z = dq.z, w = dq.w;
    double s = std::sqrt(x*x + y*y + z*z);
    
    
    if (s < 1e-12) {
        return {0, 0, 0};
    }
    
    
    double angle = 2 * std::atan2(s, w);
    
    
    double scale = angle / (dt * s);
    return {x * scale, y * scale, z * scale};
}




Trouvons deux quaternions pour lesquels sans forçage le résultat est absurde

Prenons : q0=(0,0,0,1) (identité) , q1=(0,0,−sin⁡30°,−cos⁡30°) 
Ces deux quaternions représentent la même orientation que q1′=(0,0,sin⁡30°,cos⁡30°), car q et −q codent la même rotation. Mais sans forçage du chemin court, le calcul passe par le  long chemin  (300° au lieu de 60°).

| Cas | q1​                                                                 | Avec forçage (rad/s)                     | Sans forçage (rad/s)                        |                                      |
| --- | ------------------------------------------------------------------------ | ---------------------------------------- | ------------------------------------------- | ------------------------------------------------- |
| 1   | (0,0,sin⁡30°,cos⁡30°)    | (0,0,0,5236) | (0,0,0,5236)|
| 2   | (0,0,−sin⁡30°,−cos⁡30°) | (0,0,0,5236) | (0,0,−5,2360) | 