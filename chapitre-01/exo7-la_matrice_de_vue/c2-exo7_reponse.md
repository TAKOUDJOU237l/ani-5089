## Exercice 7

la  premiere version  on a le code suivant 

#include <array>
using Mat4 = std::array<std::array<double, 4>, 4>;

Mat4 poseToMat4(const Pose& pose)
{
    
    const auto& q = pose.orientation;
    const double xx = q.x*q.x, yy = q.y*q.y, zz = q.z*q.z;
    const double xy = q.x*q.y, xz = q.x*q.z, yz = q.y*q.z;
    const double xw = q.x*q.w, yw = q.y*q.w, zw = q.z*q.w;

    Mat4 m{};
  
    m[0][0] = 1 - 2*(yy + zz);
    m[0][1] = 2*(xy - zw);
    m[0][2] = 2*(xz + yw);
    m[0][3] = pose.position.x;
    
    m[1][0] = 2*(xy + zw);
    m[1][1] = 1 - 2*(xx + zz);
    m[1][2] = 2*(yz - xw);
    m[1][3] = pose.position.y;
    
    m[2][0] = 2*(xz - yw);
    m[2][1] = 2*(yz + xw);
    m[2][2] = 1 - 2*(xx + yy);
    m[2][3] = pose.position.z;
    
    m[3][0] = 0; m[3][1] = 0; m[3][2] = 0; m[3][3] = 1;
    return m;
}


Mat4 inverserMat4(const Mat4& m);  

Pose InverserMatriciel(const Pose& pose)
{
    Mat4 m = poseToMat4(pose);
    Mat4 mInv = inverserMat4(m);

    Pose res;
    res.position = {mInv[0][3], mInv[1][3], mInv[2][3]};
  
    return res;
}

la deuxieme version avec le conjugué et la translation opposée



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