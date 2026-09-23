## Exercice 2

## Ce que le code montre

Sur une pose valide, les deux méthodes donnent exactement la même matrice, écart nul aux seize coefficients :


Inverse par inversion generale              
0.000000  0.000000 -1.000000  5.000000      
0.000000  1.000000  0.000000  2.000000      
1.000000  0.000000  0.000000 -3.000000      
0.000000  0.000000  0.000000  1.000000      

Inverse par conjugue et translation opposee
0.000000  0.000000 -1.000000  5.000000
0.000000  1.000000  0.000000  2.000000
1.000000  0.000000  0.000000 -3.000000
0.000000  0.000000  0.000000  1.000000

Puis j'ai construit une matrice dégénérée : une rotation entièrement nulle, comme le donnerait une pose corrompue en amont (capteur muet, quaternion jamais écrit, mémoire non initialisée), avec seulement la position conservée :


0.000000  0.000000  0.000000   3.000000
0.000000  0.000000  0.000000  -2.000000
0.000000  0.000000  0.000000   5.000000
0.000000  0.000000  0.000000   1.000000


Cette matrice n'est pas inversible et son déterminant est nul, la rotation ayant disparu. Passée dans l'inversion générale, voici ce qu'elle rend :


1.000000  0.000000  0.000000  0.000000
0.000000  1.000000  0.000000  0.000000
0.000000  0.000000  1.000000  0.000000
0.000000  0.000000  0.000000  1.000000


 Le programme continue comme si de rien n'était.

## À faire imaginer à la classe

Demandez si cette matrice était la vue d'un œil dans un casque, à l'instant où le capteur a un raté, qu'est-ce que le porteur verrait à cette image précise ? Laissez la classe proposer des hypothèses (un flash, un saut, un gel de l'image) avant de trancher.

## Conclusion

La caméra revient à l'origine du monde, sans rotation, l'espace d'une image. Rien ne l'explique aucun message d'erreur, aucun log, aucun signal que quelque chose a échoué. C'est exactement la catégorie de faute du chapitre : celle qui ne plante pas, et qui pourtant se voit ou plutôt, se *ressent*, puisque dans un casque cette image-là n'est pas une anomalie visuelle qu'on remarque avec l'œil critique d'un développeur, c'est un à-coup direct dans le champ de vision porté, au moment précis où l'oreille interne, elle, continue de dire que la tête n'a pas bougé.