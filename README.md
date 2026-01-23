# Drone Infinity ✈️☀️

## Présentation générale

Ce dépôt GitHub contient le travail réalisé dans le cadre d’un **projet de 3ᵉ année MSc**, initialement inspiré par le **Dassault UAV Challenge**. Le projet, nommé **Drone Infinity**, vise à poser les bases techniques et logicielles d’un drone de type *aile volante* capable de fonctionner sur des durées très longues, en s’inspirant du concept du drone-satellite **Zephyr** développé par Airbus.

⚠️ **Important** : ce projet n’a pas pour objectif de livrer un drone finalisé et opérationnel. Il s’agit volontairement d’un **projet amorce**, pensé pour être repris, amélioré et complété par de futurs étudiants.

---

## Contexte : Dassault UAV Challenge

Le Dassault UAV Challenge est une compétition d’ingénierie organisée par Dassault Aviation, destinée à confronter les étudiants à une problématique industrielle réelle : la conception et le pilotage (partiellement ou totalement) autonome d’un drone dans un environnement contraint.

Ce challenge met l’accent sur :

* le travail en équipe,
* la robustesse des choix techniques,
* l’autonomie et la stabilisation,
* la gestion de l’énergie,
* l’intégration capteurs / actionneurs / contrôle.

Bien que le projet ait été initialement pensé pour s’inscrire dans ce cadre, des contraintes de temps, de taille et de faisabilité ont conduit à s’en détacher partiellement, tout en conservant la philosophie globale.

---

## Objectif du projet Drone Infinity

L’ambition du projet est de concevoir un **drone “infini”**, c’est-à-dire capable d’alterner entre deux modes de fonctionnement :

* **Le jour** : vol plané, où l’énergie stockée est principalement utilisée pour la commande et la stabilisation, tandis que les panneaux solaires rechargent la batterie.
* **La nuit** : vol motorisé, permettant au drone de regagner de l’altitude afin de préparer la phase de plané suivante.

Le drone adopte donc une architecture de **planeur / aile volante**, très différente des drones multirotors classiques.

À l’échelle du projet étudiant, il a été décidé de :

* ne pas réaliser un drone volant complet,
* développer des **briques technologiques indépendantes** (commande, capteurs, asservissements, communication),
* documenter précisément le travail afin de faciliter une reprise ultérieure.

---

## Équipe projet

Le projet a été réalisé par une équipe de **trois étudiants en électronique et informatique**, spécialisés en mécatronique et systèmes complexes :

* **Elio Flandin** :

  * conception électronique (PCB),
  * capteurs,
  * asservissements,
  * partie contrôle bas niveau.

* **Tristan Noterman** :

  * motorisation,
  * électronique de puissance,
  * ESC et moteur brushless,
  * commande moteur.

* **Kevin Dugard** :

  * coordination et gestion du projet,
  * support logiciel,
  * intégration et validation des fonctionnalités.

---

## Cahier des charges (extrait)

Le cahier des charges initial comprend notamment :

* Commande manuelle via manette
* Acquisition de données (IMU, GPS, pression, énergie)
* Stabilisation en roulis / tangage
* Pilotage de servomoteurs
* Pilotage d’un moteur brushless
* Gestion de l’énergie
* Interface utilisateur (UART / écran)
* Sécurité et mode failsafe

À la fin du projet, plusieurs exigences sont :

* **validées**,
* **partiellement validées**,
* ou **non validées**, principalement par manque de temps ou de matériel.

---

## Architecture matérielle

### PCB (carte électronique)

Une carte électronique dédiée a été **conçue mais non fabriquée**. Elle avait pour objectif d’intégrer :

* Un **STM32H743** comme microcontrôleur principal
* Des étages d’alimentation :

  * batterie 11,1 V → 5,1 V (buck)
  * 5,1 V → 3,3 V (LDO)
* Les capteurs suivants :

  * IMU (LSM6DSR / MPU9250)
  * GPS (TESEO / NEO-6M)
  * baromètre (MPL3115A2)
  * capteur courant/tension (INA226)
* Les interfaces :

  * servomoteurs
  * ESC moteur brushless
  * caméra

Le PCB n’a pas abouti principalement à cause :

* des délais de fabrication,
* du risque d’erreur sans itération possible,
* du planning académique.

➡️ Le projet repose donc sur des **modules du commerce** pour les tests et le développement logiciel.

---

## Acquisition des données

### Centrale inertielle (IMU)

L’IMU fournit :

* accélérations sur 3 axes,
* vitesses angulaires sur 3 axes.

Fonctionnalités implémentées :

* lecture I²C des registres,
* conversion en unités physiques,
* calibration statique (offsets),
* estimation des angles de roulis et de tangage,
* préparation pour la fusion de capteurs.

### GPS

Un module **NEO-6M** a été utilisé.

Fonctionnalités logicielles :

* réception UART par interruption,
* parsing des trames NMEA (GPGGA, GPRMC),
* stockage structuré des données (latitude, longitude, altitude, vitesse, satellites).

⚠️ Limitation matérielle : l’antenne utilisée étant de faible qualité, aucun fix GPS exploitable n’a été obtenu.

### Extensions envisagées

* roue codeuse pour la vitesse moteur,
* capteur de courant moteur,
* tube de Pitot,
* caméra pour vision et mapping.

---

## Communication des données

Deux modes ont été explorés :

* **UART vers PC** (PuTTY) : affichage temps réel des données capteurs.
* **Affichage embarqué** : écran OLED SH1106 (code entièrement fonctionnel).

---

## Commande et pilotage

### Chaîne de commande

1. Manette (type Xbox 360)
2. Script Python (pygame + pyserial)
3. Transmission UART (trame binaire)
4. STM32
5. Génération PWM
6. Servomoteurs / ESC

### Trame binaire

* Début de trame (0xAA)
* Throttle, roll, pitch, yaw (int16)
* Flags
* Checksum

La trame est vérifiée côté STM32 pour garantir l’intégrité des données.

---

## Servomoteurs et PWM

* Fréquence PWM : 50 Hz
* Largeur d’impulsion :

  * 1000 µs → butée min
  * 1500 µs → neutre
  * 2000 µs → butée max

Les servomoteurs commandent les gouvernes de l’aile volante.

---

## Moteur brushless et ESC

Une tentative de pilotage d’un moteur brushless a été réalisée :

* PWM similaire aux servos
* Séquences d’armement

Résultat :

* ESC reconnu,
* moteur non fonctionnel.

Hypothèses :

* séquence de calibration incorrecte,
* alimentation inadaptée,
* fréquence PWM incompatible.

Le code est néanmoins prêt à être repris et finalisé.

---

## Asservissements

### Asservissement d’attitude

* Estimation des angles (roll / pitch)
* Correcteur PID (PD implémenté)
* Saturation des commandes
* Conversion angle → PWM
* Mixage des gouvernes (aile volante)

La chaîne logicielle est complète mais n’a pas été testée en conditions réelles de vol.

### Asservissement moteur (concept)

Architecture envisagée :

* boucle courant interne,
* boucle vitesse externe,
* PWM triphasée.

---

## Sécurité et failsafe

Un module de démonstration a été développé sur breadboard incluant :

* joystick,
* moteur DC,
* écran OLED,
* LEDs,
* buzzer,
* Bluetooth.

Fonctionnalités :

* mode **LOCKED / ARMED**,
* indication sonore et visuelle,
* gestion de perte de signal,
* base pour un retour à position GPS.

---

## Conclusion

Le projet **Drone Infinity** pose des bases solides pour un drone à longue endurance :

* architecture claire,
* briques logicielles réutilisables,
* documentation détaillée,
* nombreuses pistes d’amélioration.

Il est volontairement **incomplet**, afin de servir de support pédagogique et technique pour de futurs projets.

🚀 **Reprendre ce projet, c’est gagner plusieurs mois de travail d’initialisation.**

---

## Licence

Projet académique – librement réutilisable à des fins pédagogiques.
