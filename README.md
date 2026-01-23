Voici une proposition de README.md exhaustive et détaillée pour votre dépôt GitHub, élaborée à partir des éléments techniques et organisationnels de votre rapport de projet.

🛰️ Drone Infinity - Dassault UAV Challenge
Ce projet a été réalisé par une équipe d'étudiants en 3ème année de spécialité Mécatronique et Systèmes Complexes (MSC) à l'ENSEA. L'objectif initial s'inscrit dans le cadre du Dassault UAV Challenge, une compétition d'ingénierie aéronautique exigeante.

Le concept Infinity propose la conception d'un drone de type "planeur solaire" capable d'une autonomie théoriquement illimitée en alternant des phases de vol plané et de propulsion motorisée selon un cycle jour/nuit.

📋 Présentation du Concept "Infinity"
Inspiré du drone-satellite Zephyr d'Airbus, le projet repose sur une stratégie énergétique spécifique:


Stratégie diurne : Le drone utilise la portance pour planer tout en minimisant la consommation énergétique (uniquement pour le contrôle directionnel). Simultanément, des panneaux solaires rechargent les batteries.


Stratégie nocturne : Le drone active son moteur principal pour regagner de l'altitude et se maintenir grâce à l'énergie stockée durant la journée.

🛠️ Architecture Matérielle (Hardware)
Le système repose sur un microcontrôleur haute performance STM32H743VIT6.

1. PCB & Énergie
Bien que des modules séparés aient été utilisés pour le prototype final en raison de contraintes de délais, une carte électronique dédiée a été conçue sous KiCad.


Alimentation : Abaisseur de tension (Buck) L4973D5.1 pour passer de la batterie 11.1V (LiPo) vers 5.1V, puis régulateur LDO LD1117D33TR pour le 3.3V des capteurs.


Mesure de puissance : Capteur INA226 pour surveiller en temps réel la tension et le courant délivrés par les panneaux solaires.

2. Capteurs & Instrumentation

Centrale Inertielle (IMU) : Module GY-91 (intégrant un MPU9250) communiquant en I2C pour mesurer l'accélération et la vitesse angulaire sur 3 axes.


Positionnement : GPS GY-NEO 6M (UART) pour le mapping et le retour au point de départ (RTL).


Interface Locale : Écran OLED géré par un driver SH1106 pour l'affichage de l'état du système (vitesse, mode "Armed/Locked", état Bluetooth).

💻 Logiciel & Contrôle (Software)
1. Interface de Commande PC (Python)
Le pilotage s'effectue via un script Python utilisant la bibliothèque pygame pour lire les entrées d'une manette (type Xbox 360).


Communication : Envoi d'une trame binaire structurée (11 octets) via UART à 115200 bauds.


Sécurité des données : Implémentation d'un Checksum (somme de contrôle) côté Python et STM32 pour valider l'intégrité de chaque commande reçue.

2. Algorithmes de Stabilisation
Le firmware STM32 intègre une boucle d'asservissement complète:


Estimation d'attitude : Calcul des angles de roulis (roll) et de tangage (pitch) à partir des données IMU et d'un filtre complémentaire.


Correcteur PID : Régulation en position angulaire pour stabiliser le drone face aux perturbations.


Mixage de gouvernes : Les commandes de roulis et tangage sont combinées pour piloter les deux servomoteurs des ailes.

3. Sécurité (Failsafe)
Un mode de sécurité critique a été conçu pour déclencher un atterrissage d'urgence ou la mise en sécurité des moteurs en cas de perte de signal radio ou de batterie faible.

📂 Structure du Projet
Plaintext
├── Firmware/             # Code source C (STM32CubeIDE)
│   ├── Drivers/          # Drivers capteurs (IMU, GPS, OLED)
│   ├── Src/              # Gestion PID, PWM et interruptions UART
├── PC_Client/            # Script Python (Pygame & Serial)
├── Hardware/             # Schémas et fichiers KiCad du PCB
└── Docs/                 # Rapport complet et fiches techniques
🚀 État de l'avancement & Perspectives
Le projet constitue une base robuste pour les promotions futures.


Validé : Communication manette-PC-STM32, acquisition IMU calibrée, génération PWM pour servos, affichage OLED.


À poursuivre : Finalisation du routage et fabrication du PCB dédié, optimisation de l'antenne GPS, intégration complète du moteur brushless avec asservissement de vitesse.

👥 Équipe (Groupe MSC)

Elio FLANDIN : Mesures, conception PCB & Asservissement.


Tristan NOTERMAN : Motorisation, puissance & énergie.


Kevin DUGARD : Gestion de projet & Développement Software (PC/STM32).
