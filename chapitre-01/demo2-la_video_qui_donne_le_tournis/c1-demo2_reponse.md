# Exercice 2



## Le conflit oreille interne et œil, expliqué simplement

Normalement, l'œil et l'oreille interne racontent la même histoire en même temps. L'œil voit le monde bouger autour de vous quand vous tournez la tête. L'oreille interne, le système vestibulaire situé dans l'oreille, sent directement l'accélération de votre tête, sans passer par un écran et sans délai. En temps normal, les deux témoignages arrivent ensemble, et le cerveau les fusionne sans y penser.

Avec le retard que vous venez de montrer, ce n'est plus le cas. L'oreille dit « je tourne maintenant », mais l'œil montre l'image d'il y a 60, 100 ou 150 millisecondes, donc une scène qui n'a pas encore bougé ou qui bouge en retard. Le cerveau reçoit alors deux témoignages contradictoires sur le même événement : j'ai bougé, mais je ne vois pas encore le résultat. C'est exactement le genre de contradiction sensorielle que le corps interprète, depuis très longtemps sur le plan évolutif, comme un signe d'empoisonnement, puisque les toxines perturbent l'équilibre sans perturber la vue de la même façon. La réaction est donc physique, nausée et malaise, et pas seulement un inconfort visuel.

Demander ensuite qui n'a rien senti dans la salle est important parce que ça révèle une chose essentielle : la sensibilité à ce conflit n'est pas la même pour tout le monde. Certains la ressentent dès 40 millisecondes, d'autres pas même à 150 ou 200 millisecondes sur un simple écran, et l'écart est encore plus grand en casque. Cela a deux conséquences directes pour vous, en tant que développeur. D'abord, vous ne pouvez jamais vous fier à votre propre ressenti seul pour juger si votre application est confortable ; si vous êtes vous-même peu sensible, vous pourriez livrer quelque chose d'inconfortable pour la majorité sans jamais vous en rendre compte, puisque ce genre de défaut ne plante pas. Ensuite, c'est précisément pour cela qu'on teste sur plusieurs personnes et qu'on écoute leurs seuils individuels plutôt que de fixer un seul chiffre universel : le budget de 20 ms n'est pas une moyenne confortable, c'est une marge de sécurité pensée pour couvrir aussi les personnes les plus sensibles de la salle.

