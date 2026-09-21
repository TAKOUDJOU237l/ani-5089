# Exercice 1 les trois cadances



 72 Hz

la duree  d'une image  = 1000 / frequences en Hz

 la duree d une image :
1000 / 72 = 13.9 ms

il faut retirer les 8 ms reservees aux capteurs , a la transmision ,a la composition et a l affichage 

Temps restant pour le code 
13.9 - 8 = 5.9 ms

 90 Hz


la duree d une image :

1000 / 90 = 11.1 ms

Temps restant pour le code 
11.1 - 8 = 3.1 ms


120 Hz

la duree d une image :

1000 / 120 = 8.3 ms

Temps restant pour le code 
8.3 - 8 = 0.3 ms

conclusion 

plus la frequence d affichage augmente , plus la duree disponible pour chaque image diminue. Apres aavoir retiré 8ms utilisee par les capteurs; la transmissions , la composition, et l'affichage ,il reste 5.9ms a 7.5hz , 3.1 ms a 90 hz et seulement 0.3ms a 120 hz pour le code. cela montre qu'en realité virtuelle, le programe doit etre tre rapide et respecter l'écheance de chaque image