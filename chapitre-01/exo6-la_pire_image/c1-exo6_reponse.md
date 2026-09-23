# Exercice 6 :

## le code

#include <SFML/Graphics.hpp>
#include <chrono>
#include <iostream>

int main()
{
    sf::RenderWindow fenetre(sf::VideoMode(400, 300), "budget des images");

    const int NB_IMAGES = 1000;
    const double SEUIL_MS = 11.0;

    double pireImageMs = 0.0;
    int nbImagesTropLongues = 0;

    for (int i = 0; i < NB_IMAGES && fenetre.isOpen(); i++)
    {
        sf::Event evenement;
        while (fenetre.pollEvent(evenement))
        {
            if (evenement.type == sf::Event::Closed) fenetre.close();
        }

        auto debut = std::chrono::high_resolution_clock::now();

        fenetre.clear(sf::Color::Black);
        fenetre.display();

        auto fin = std::chrono::high_resolution_clock::now();
        double dureeMs = std::chrono::duration<double, std::milli>(fin - debut).count();

        if (dureeMs > pireImageMs) pireImageMs = dureeMs;
        if (dureeMs > SEUIL_MS) nbImagesTropLongues++;
    }

    std::cout << "Pire image : " << pireImageMs << " ms\n";
    std::cout << "Images au dessus de " << SEUIL_MS << " ms : "
              << nbImagesTropLongues << " sur " << NB_IMAGES << "\n";

    fenetre.close();
    return 0;
}

### resultat obtenu on aura:
Pire image qui est egale a 0.800137 ms
Images au dessus de 11 ms : 0 sur 1000

### Est ce que le programme tiendra dans un casque ?
Techniquement oui, très largement , on as 0,8 ms de pire cas tient sans effort dans un budget de 8,3 ms à 120 Hz. Mais il faut être honnête sur ce que ce chiffre représente vraiment, car ce programme ne fait rien : il efface l'écran et l'affiche, sans dessiner de scène, sans calculer de physique, sans charger de ressources, ce qui en fait le meilleur cas possible, presque un cas dégénéré, un peu comme mesurer la vitesse d'une voiture au point mort. L'exercice précédent listait précisément ce qui compose le budget réel, à savoir les capteurs, la transmission, la décision et le dessin de l'application, le compositeur, puis l'écran, et ce programme ne représente que le dessin à sa forme la plus vide. La vraie question que pose le chapitre n'est donc pas de savoir si un écran vide tient dans 11 ms, ce qui est toujours vrai, mais si votre scène réelle, avec sa géométrie, ses textures et sa physique, tient dans ce budget à chaque image, et non en moyenne. Pour répondre sérieusement à cette question, il faudrait relancer cette même mesure en remplaçant fenetre.clear() par le rendu réel de votre projet.