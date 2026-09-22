# Seuil de perception du retard, sur écran ordinaire

## Le programme



#include <SFML/Graphics.hpp>
#include <deque>
#include <string>
#include <cmath>

struct Echantillon {
    sf::Int32 tempsMs;  
    sf::Vector2i pos;    
};

int main() {
    const unsigned int LARGEUR = 900, HAUTEUR = 650;
    sf::RenderWindow fenetre(sf::VideoMode(LARGEUR, HAUTEUR),
                              "Retard souris (0-200 ms)");
    fenetre.setFramerateLimit(120);

    sf::Font police;
    bool policeChargee = police.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");

    sf::Clock horloge;                 
    std::deque<Echantillon> historique;

    int retardMs = 0;                  
    const int PAS = 10;                
    const int RETARD_MAX = 200;

    sf::CircleShape curseurReel(4.f);
    curseurReel.setFillColor(sf::Color(120, 120, 120));
    curseurReel.setOrigin(4.f, 4.f);

    sf::CircleShape curseurRetarde(10.f);
    curseurRetarde.setFillColor(sf::Color(220, 30, 30));
    curseurRetarde.setOrigin(10.f, 10.f);

    while (fenetre.isOpen()) {
        sf::Event evt;
        while (fenetre.pollEvent(evt)) {
            if (evt.type == sf::Event::Closed)
                fenetre.close();
            if (evt.type == sf::Event::KeyPressed) {
                if (evt.key.code == sf::Keyboard::Escape)
                    fenetre.close();
                if (evt.key.code == sf::Keyboard::Up)
                    retardMs = std::min(RETARD_MAX, retardMs + PAS);
                if (evt.key.code == sf::Keyboard::Down)
                    retardMs = std::max(0, retardMs - PAS);
            }
        }

        
        sf::Int32 maintenant = horloge.getElapsedTime().asMilliseconds();
        sf::Vector2i posSouris = sf::Mouse::getPosition(fenetre);
        historique.push_back({maintenant, posSouris});

        
        while (!historique.empty() &&
               historique.front().tempsMs < maintenant - RETARD_MAX - 50)
            historique.pop_front();

       
        sf::Int32 cible = maintenant - retardMs;
        sf::Vector2i posRetardee = posSouris; 
        for (const auto &e : historique) {
            if (e.tempsMs <= cible)
                posRetardee = e.pos;   
            else
                break;
        }

        
        fenetre.clear(sf::Color(25, 25, 30));

        curseurReel.setPosition((float)posSouris.x, (float)posSouris.y);
        fenetre.draw(curseurReel);

        curseurRetarde.setPosition((float)posRetardee.x, (float)posRetardee.y);
        fenetre.draw(curseurRetarde);

        if (policeChargee) {
            sf::Text texte;
            texte.setFont(police);
            texte.setCharacterSize(22);
            texte.setFillColor(sf::Color::White);
            texte.setPosition(15.f, 15.f);
            texte.setString("Retard : " + std::to_string(retardMs) +
                             " ms   (Haut/Bas pour regler, Echap pour quitter)");
            fenetre.draw(texte);
        }

        fenetre.display();
    }

    return 0;
}



## Les cinq seuils

| Participant | Seuil perçu | Remarque notée pendant le test |
|---|---|---|
| Personne 1 | 40 ms | Détecté vite, dit « ça traîne un peu » en faisant des cercles rapides |
| Personne2 | 70 ms | Ne remarque rien en mouvement lent, seulement sur les allers-retours brusques |
| Personne 3 | 50 ms | Très sensible, joue à des jeux vidéo régulièrement |
| Personne 4 | 90 ms | Peu habituée à l'informatique, seuil plus élevé |
| Personne 5 | 60 ms | Seuil intermédiaire, remarque surtout sur les changements de direction |

**Moyenne : 62 ms** (min 40, max 90).

## Comparaison au budget de 20 ms

Le budget mouvement vers le photon d'un casque vise à rester sous ~20 ms. Ici, sur un écran ordinaire, le retard doit atteindre en moyenne **62 ms**  trois fois plus avant d'être remarqué.Trois raisons expliquent cet écart :

- **Un seul canal sensoriel est en jeu ici.** Sur écran, on compare une image vue à une intention de mouvement de la main : c'est un simple retour visuomoteur. Dans un casque, le retard oppose deux sens à la fois  la vue et l'oreille interne (vestibulaire), qui, elle, signale le mouvement de la tête en temps réel, sans latence. C'est ce **conflit sensoriel** entre deux canaux normalement synchronisés qui provoque le malaise, pas seulement un décalage visuel gênant.
- **L'échelle du mouvement diffère.** La main sur une souris ne déplace le curseur que de quelques centimètres à l'écran ; la tête, elle, fait pivoter l'intégralité du champ visuel. Le même retard produit donc un décalage visuel bien plus grand, et donc plus détectable, en VR.
- **Le champ de vision est total.** Sur écran, l'image retardée n'occupe qu'une petite partie du champ visuel, entourée d'un environnement stable (le bureau, le clavier) qui sert de référence immobile. En casque, l'image retardée occupe tout le champ visuel : il n'y a plus de référence stable pour « rattraper » l'erreur, ce qui abaisse fortement le seuil de perception.

conclusion 

 le seuil mesuré ici (~60 ms) est celui d'un simple inconfort visuel isolé. Le seuil en VR est plus bas (~20 ms) parce qu'au-delà, ce n'estplus une gêne visuelle, mais un conflit avec l'équilibre la même catégorie de défaut « qui ne plante pas mais se sent », évoquée dans le chapitre 2.
