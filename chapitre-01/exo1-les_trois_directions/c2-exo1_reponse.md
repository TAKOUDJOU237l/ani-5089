# Exercice 1 les trois fonctions


#include <iostream>
#include <iomanip>

struct Vecteur
{
    double x;
    double y;
    double z;
};

Vecteur Avant()
{
    return {0.0, 0.0, -1.0};
}

Vecteur Haut()
{
    return {0.0, 1.0, 0.0};
}

Vecteur Droite()
{
    return {1.0, 0.0, 0.0};
}

double ProduitScalaire(const Vecteur& a, const Vecteur& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z + 0.0;
}

int main()
{
    Vecteur point;

    std::cin >> point.x >> point.y >> point.z;

    std::cout << std::fixed << std::setprecision(4);

    std::cout << ProduitScalaire(point, Avant()) << '\n';
    std::cout << ProduitScalaire(point, Haut()) << '\n';
    std::cout << ProduitScalaire(point, Droite()) << '\n';

    return 0;
}

### Exemple d excecution 

point choisi (0.0, 0.0, -2.0)

sortie attendu (2.0000 , 0.0000 , 0.0000 )