# Exercice 4

## Démonstration en classe : trois volontaires, retard réglable

Le programme à retard réglable a été projeté en plein écran. Trois
volontaires sont passés un par un, devant toute la classe. Pour chacun, le
retard a été augmenté par paliers de 10 ms pendant qu'il bougeait librement
la souris, sans lui indiquer à l'avance à quel palier s'attendre, et le
seuil noté au tableau dès qu'il annonçait sentir un décalage.

### Les trois seuils

| Volontaire | Seuil relevé | Remarque |
|---|---|---|
| Volontaire 1 | 40 ms | Réagit vite, dit sentir un « traînage » dès les premiers paliers |
| Volontaire 2 | 80 ms | Ne remarque rien avant un mouvement brusque de la souris |
| Volontaire 3 | 55 ms | Seuil intermédiaire, hésite un palier avant de confirmer |

Écart observé : de 40 à 80 ms, soit un facteur deux entre le volontaire le
plus sensible et le moins sensible.

### Ce que cela implique pour le budget d'une image

L'écart entre les trois seuils est la donnée qui compte, plus que chaque
chiffre pris isolément. Avec seulement trois personnes, on obtient déjà un
facteur deux entre le seuil le plus bas et le plus haut, ce qui confirme
qu'il est impossible de deviner à l'avance qui sera sensible et qui ne le
sera pas, rien qu'en regardant les gens.

Un budget d'image calé sur le volontaire 2 (80 ms) livrerait une expérience
manifestement inconfortable pour le volontaire 1. C'est ce raisonnement,
répété à grande échelle sur une population entière, qui justifie de fixer
le budget d'une image bien en dessous de la moyenne observée : le budget
d'un casque, à 20 ms, n'est pas pensé pour l'utilisateur moyen mais pour
rester sous le seuil même des utilisateurs les plus sensibles de la salle.

*Note : les trois seuils ci-dessus sont donnés à titre d'illustration du
protocole ; à remplacer par les vrais chiffres relevés en classe.*