# Exercice  demo1

# Activité au tableau : le budget de 20 ms comme une barre

![Photo de la démonstration au tableau](file:///C:/Users/LENOVO/Desktop/bobo/chapitre-01/demo1-le_budget_au_tableau/images.jpg)

#

## Étape 1  Tracer la référence

Tracez une barre horizontale unique, graduée de 0 à 20 ms. Choisissez une échelle simple à mesurer à l'œil ou à la règle, par exemple un millisecondes pour quatre centimètres, ce qui donne une barre de quatre vingt centimètres, ou bien un millisecondes par carreau sur un tableau quadrillé, soit vingt carreaux. Écrivez seulement "0 ms" à gauche et "20 ms" à droite pour l'instant, rien d'autre.

## Étape 2 Faire placer les cinq étapes, cas optimiste

Distribuez les cinq cartons. Les capteurs mesurent le mouvement en une à deux millisecondes. Le système transmet la mesure en une à trois millisecondes. L'application décide et dessine en cinq à onze millisecondes. Le compositeur assemble en une à deux millisecondes. L'écran affiche la ligne en deux à cinq millisecondes.Demandez à cinq élèves de venir, dans cet ordre, placer et tracer leur segment à la borne basse de leur plage, bout à bout sur la barre, chacun démarrant où le précédent s'est arrêté. Ils doivent mesurer, pas estimer à l'œil.Le résultat attendu est que la barre est occupée jusqu'à dix millisecondes, exactement la moitié. C'est le moment de faire réagir la classe en demandant à qui appartient la moitié restante. La réponse instinctive est souvent que c'est au code, puisque c'est la plus grosse tranche, mais c'est faux : cette moitié n'est déjà plus disponible en pratique, elle représente simplement la marge qui disparaît dès qu'on quitte le cas le plus favorable. C'est la phrase du chapitre à faire résonner ici : votre code n'a pas vingt millisecondes, il en a une dizaine.

## Étape 3  Étendre à la borne haute, sans effacer

Sans effacer les traits précédents, demandez aux mêmes cinq élèves d'étendre leur segment jusqu'à la borne haute de leur plage, toujours dans l'ordre, chacun repartant de là où le précédent s'arrête maintenant.Le résultat attendu est que le trait dépasse physiquement la marque des vingt millisecondes, puisque la somme des maximums fait vingt trois millisecondes. Laissez ce dépassement bien visible, la craie sortant du cadre tracé au tableau. Faites réagir la classe sur ce que ce dépassement signifie concrètement dans un casque : ce n'est pas un ralentissement qu'on remarque à peine, c'est un rendez vous manqué. Le matériel affiche une image au moment prévu, que votre programme soit prêt ou non.

## Étape 4  Isoler ce qui reste pour le code
Faites calculer collectivement, au tableau, la somme des quatre étapes qui ne sont pas le code, c'est à dire les capteurs, la transmission, le compositeur et l'écran. Au mieux, cela donne un plus un plus un plus deux, soit cinq millisecondes. Au pire, cela donne deux plus trois plus deux plus cinq, soit douze millisecondes.Demandez ensuite à la classe : si les quatre autres étapes sont toutes à leur pire cas, combien reste t il pour votre application sur les vingt millisecondes ? La réponse est vingt moins douze, soit huit millisecondes, c'est à dire moins que le pire cas que le code s'autorise lui même, qui est de onze millisecondes.Faites réagir sur ce paradoxe : l'étape la plus large de tout le budget, celle qui va de cinq à onze millisecondes avec un écart de six millisecondes, le plus grand de toutes, est censée être la mieux maîtrisée puisque c'est la seule que le développeur écrit lui même. Les quatre autres étapes sont subies, celle là est la seule sur laquelle on peut agir, et c'est justement celle qui varie le plus.

## Question de clôture à poser à la classe

Une image sur cent qui prend le double se voit. Demandez à la classe : sur cette barre, à quoi ressemblerait cette image là ? Faites visualiser un sixième trait, dessiné une seule fois, qui dépasse largement les vingt trois millisecondes. C'est l'image que le pire cas mesuré en labo ne montre pas forcément, mais que l'oreille interne de l'utilisateur, elle, remarque immédiatement.