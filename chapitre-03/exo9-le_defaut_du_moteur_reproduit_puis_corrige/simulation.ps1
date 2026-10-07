# SIMULATION (pas une mesure) : rejoue un flux d'evenements ecrit a la main,
# deux fois : lecture naive (derniere valeur, jamais remise a zero) et
# accumulateur (somme, remis a zero a chaque image). Une chaine = une image,
# les evenements sont "dx,dy" separes par des points-virgules.
$dossier = $PSScriptRoot
$frames = @("2,1;3,1;4,2", "5,2;6,3;7,3;8,4", "8,4;9,4;7,3;6,3", "5,2;4,2;3,1", "2,1;1,0", "", "", "", "", "")
$nx = 0; $ny = 0
$avant = @(); $apres = @()
foreach ($f in $frames) {
  $ax = 0; $ay = 0
  if ($f -ne "") {
    foreach ($e in $f.Split(";")) {
      $p = $e.Split(",")
      $dx = [int]$p[0]; $dy = [int]$p[1]
      $nx = $dx; $ny = $dy
      $ax += $dx; $ay += $dy
    }
  }
  $avant += "$nx $ny"
  $apres += "$ax $ay"
}
Set-Content -Encoding ascii "$dossier\avant.txt" $avant
Set-Content -Encoding ascii "$dossier\apres.txt" $apres
