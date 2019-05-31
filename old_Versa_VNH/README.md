# DMXbimoteur

DMX512 control of 2 DC motors, for the GroupeF company.

La carte DMXbimoteur permet de contrôler la vitesse de 2 moteurs CC de 12V à 24 V par une connexion DMX512.

### assignation du canal DMX

Le canal DMX de la carte doit d'abord être configuré de la façon suivante :

1. éteindre tous les canaux DMX sur cet univers
2. régler le canal désiré à 100% (valeur 255)
3. régler le canal suivant à 100% (valeur 255) (optionnel : permet d'éviter la rotation du  moteur A ; voir **fonctionnement**)
4. appuyer sur le bouton poussoir ("channel select switch" sur le plan de câblage)

Le canal choisi est ainsi mémorisé dans la carte (EEPROM).

### fonctionnement

La carte utilise 4 canaux DMX : 

* canal1 (le canal précédemment configuré): moteur A envers
* canal2 = canal1 + 1 : moteur A endroit
* canal3 = canal1 + 2 : moteur B envers
* canal4 = canal1 + 3 : moteur B endroit

**exemple :** si on configure la carte sur le canal 100, on aura:

* canal 1 = 100
* canal 2 = 101
* canal 3 = 102
* canal 4 = 103


La vitesse des moteurs est calculée comme suit :

` vitesse A = 100% * (valeur[canal 2] - valeur[canal 1])/255 `

` vitesse B = 100% * (valeur[canal 4] - valeur[canal 3])/255 `

**exemples :** 

`valeur[canal 1] = 000 , valeur[canal 2] = 000 => vitesse A = 0% `

`valeur[canal 1] = 255 , valeur[canal 2] = 000 => vitesse A = -100% `

`valeur[canal 1] = 255 , valeur[canal 2] = 255 => vitesse A = 0% `

`valeur[canal 1] = 000 , valeur[canal 2] = 255 => vitesse A = +100% `

`valeur[canal 1] = 000 , valeur[canal 2] = 128 => vitesse A = +50% `

## sources

Le code source est disponible à l'adresse suivante :

https://github.com/MetaluNetProjects/DMXbimoteur


**metalu.net**

GNU GENERAL PUBLIC LICENSE Version 3