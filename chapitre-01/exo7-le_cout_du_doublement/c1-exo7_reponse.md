# Exercice 7


## Mesure

En relançant le même programme, la pire image mesurée sur les mille passe de 0,800 ms à 0,948 ms d'une exécution à l'autre, ce qui montre déjà que même un rendu vide n'est pas parfaitement stable d'une mesure à l'autre. Je retiens 0,948418 ms comme le coût du rendu seul pour cette estimation.

## Estimation

Puisque le rendu est l'élément qui se double par œil, un rendu fait deux fois coûterait environ 2 fois 0,948418 ms, soit à peu près 1,90 ms. En reprenant le budget de 11 ms attribué à l'application dans l'exercice précédent, il resterait environ 11 moins 1,90, soit à peu près 9,10 ms pour tout ce qui n'est pas doublé, c'est à dire la logique du jeu, la physique et la décision de ce qu'il faut dessiner.

## Conclusion

Ce qu'il faudrait réduire en priorité, ce n'est pas la logique mais le rendu lui même, parce que c'est la seule partie du budget qui se paie deux fois : chaque milliseconde gagnée sur le dessin de la scène en fait gagner deux sur le total, alors qu'une milliseconde gagnée sur la physique ou sur la décision n'en fait gagner qu'une. Et il faut garder à l'esprit que ce chiffre de 0,948 ms reste optimiste, puisqu'il correspond à un écran qui n'affiche rien : une vraie scène, avec géométrie, textures et éclairage, prendra nécessairement plus de temps à dessiner, donc coûtera plus cher une fois doublée, donc laissera d'autant moins de marge pour le reste.