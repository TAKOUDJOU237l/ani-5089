# Exercice

Exécuté réellement. Le montage : deux orientations vraies à 10° et 11° autour de z, un delta physique d'exactement 1°, un échantillonnage à 90 Hz. La seconde quaternion est ensuite renvoyée sous sa forme négée (-x,-y,-z,-w), ce qui représente exactement la même rotation, mais c'est ce que fait un vrai tracker d'une image à l'autre sans qu'on le lui demande.

## Sans le forçage du chemin court


On as une vitesse angulaire de 32310.000 deg/s


Un delta d'1° lu comme 32 310°/s, c'est à dire l'équivalent d'un tour complet plus un peu, ramené à une fraction de seconde. Le mécanisme est direct : conjuguer(q1) * q2  mesure l'angle entre deux quaternions, mais rien ne garantit qu'ils pointent du même côté de la double couverture. Ici w du quaternion relatif se retrouve proche de -1 au lieu de proche de 1, donc acos renvoie un angle proche de 2π plutôt qu'un angle proche de zéro. Le calcul ne plante pas, ne produit aucun avertissement, et rend un nombre parfaitement plausible en apparence.

## Avec le forçage du chemin court

Les trois lignes ajoutées, avant le second calcul :

## le code est le suivant

double produit = q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w;
if (produit < 0.0)
    q2 = { -q2.x, -q2.y, -q2.z, -q2.w };


Le produit scalaire dit simplement si les deux quaternions regardent du même côté de la sphère double. S'il est négatif, on renvoie q2 de l'autre côté avant tout calcul d'angle et  l'opération inverse exacte de ce qui avait corrompu la mesure.


Vitesse angulaire : 90.000 deg/s


