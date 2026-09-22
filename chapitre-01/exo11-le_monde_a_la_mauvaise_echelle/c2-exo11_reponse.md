# Exercice 11


#include <iostream>
#include <iomanip>
#include <string>

struct Dimensions {
    double longueur;
    double largeur;
    double hauteur;
};

void afficherAvecFacteur(const std::string& nom, const Dimensions& d, double facteur) {
    std::cout << "  " << nom << " :\n";
    std::cout << "    Longueur : " << std::fixed << std::setprecision(2)
              << d.longueur * facteur << " m\n";
    std::cout << "    Largeur  : " << d.largeur * facteur << " m\n";
    std::cout << "    Hauteur  : " << d.hauteur * facteur << " m\n";
}

int main() {
    
    Dimensions salle = {5.0, 4.0, 2.5};
    Dimensions table = {1.8, 0.9, 0.75};
    Dimensions chaise = {0.5, 0.5, 0.85};
    Dimensions armoire = {1.2, 0.6, 2.0};

    double facteur;
    std::cout << "Entrez le facteur d'Ú©chelle : ";
    std::cin >> facteur;

    std::cout << "\n Dimensions avec facteur " << facteur << " \n\n";

    std::cout << "Salle :\n";
    afficherAvecFacteur("Salle", salle, facteur);

    std::cout << "\nMobilier :\n";
    afficherAvecFacteur("Table", table, facteur);
    afficherAvecFacteur("Chaise", chaise, facteur);
    afficherAvecFacteur("Armoire", armoire, facteur);

    return 0;
}


Decriptions de  la salle a 3 personnes pour 3 facteur differents

nous avons une salle (5 m × 4 m × 2,5 m, avec table, chaise, armoire) à trois personnes, chacune voyant les dimensions multipliées par un facteur différent sans le connaître.

Personne 1 avec un facteur de 0,6
C'est une petite pièce, presque une chambre d'enfant. La salle fait genre 3 mètres sur 2,40, avec un plafond assez bas, genre 1,50 m. On se sent un peu écrasé. La table est minuscule, comme une table d'appoint, même pas un mètre de long. Les chaises ressemblent à des tabourets pour tout-petits, et l'armoire arrive à peine au-dessus de ma tête. On dirait une maison de poupée. 

Personne 2 avec un facteur de 1,0
 C'est une salle de taille normale, genre un petit salon ou une chambre standard. Environ 5 mètres sur 4, plafond à 2,50 m, ça passe. La table fait une taille de salle à manger classique, genre 1,80 m, on peut mettre 6 personnes autour. Les chaises sont normales, hauteur standard. L'armoire est haute, elle arrive au plafond ou presque. Rien ne choque, c'est cohérent avec un vrai appartement. 


Personne C  avec un facteur de 1,8
 Là c'est immense, on dirait un loft ou une salle de réunion. La pièce fait presque 9 mètres de long sur 7 de large, avec un plafond de plus de 4 mètres, ça fait cathédrale. La table est gigantesque, comme une table de conférence, on pourrait asseoir 12 personnes facilement. Les chaises sont énormes, presque des fauteuils. L'armoire est démesurée, elle dépasse largement ma tête, on se sent tout petit dedans. C'est impressionnant mais un peu oppressant. 

conclusion
lorsque le facteur  est inferieure a 1  le monde paraît réduit par consequent l'utilisateur se sent géant ,si le facteur est egale a  1 tout paraît  normal par consequent cohérent avec l'expérience réelle et au final si le facteur est superieure a 1  le monde paraît agrandi par consequent l'utilisateur se sent petit.

Aucune des trois personnes n'a deviné le facteur numérique, mais toutes ont ressenti l'erreur d'échelle, ce qui confirme le point du cours : en VR, une mauvaise échelle ne plante pas, elle se sent.