# exercice 2: le tableau des budgets 


etape | ordre de grandeur (cours) | source |
|---|---|---|
| Les capteurs mesurent le mouvement | 1 à 2 ms | Bosch Sensortec, communiqué constructeur sur l'IMU BMI085 (gyroscope et accéléromètre conçu pour VR/AR) [bosch-sensortec.com](https://www.bosch-sensortec.com/en/news/imu-bmi085-for-virtual-and-augmented-reality-applications.html) 
| Le système transmet la mesure | 1 à 3 ms | *Introuvable.* | 
| Votre application décide et dessine | 5 à 11 ms | Meta Horizon OS Developer Docs, doc constructeur officielle  [developers.meta.com/.../os-missed-frames](https://developers.meta.com/horizon/documentation/unity/os-missed-frames/) ; blog technique Meta — [developers.meta.com/.../how-to-optimize-your-oculus-quest-app](https://developers.meta.com/horizon/blog/how-to-optimize-your-oculus-quest-app-w-renderdoc-quest-hardware-and-software-offerings/) |
| Le compositeur assemble | 1 à 2 ms | Brevet Google (documentation technique constructeur)  [image-ppubs.uspto.gov, brevet 10499042](https://image-ppubs.uspto.gov/dirsearch-public/print/downloadPdf/10499042) ; Meta Horizon OS Developer Docs, Performance HUD  [developers.meta.com/.../dg-hud](https://developers.meta.com/horizon/documentation/native/pc/dg-hud/) |
| L'écran affiche la ligne | 2 à 5 ms | Article technique RoadToVR sur un écran OLED Google/LG pour VR  [roadtovr.com](https://www.roadtovr.com/google-lg-detail-upcoming-1443-ppi-oled-vr-display/) | 
