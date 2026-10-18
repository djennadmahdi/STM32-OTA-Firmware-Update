# Application STM32 — Vérification des mises à jour OTA

Ce dossier contient le fichier `main.c` de l'application embarquée développée pour le microcontrôleur STM32F446RE.

## Fonctionnement

L'application utilise un timer TIM3 pour déclencher périodiquement une vérification des mises à jour OTA toutes les 30 secondes.

La fonction `check_OTA_GSM()` communique avec le module SIM7600E via UART6 à l'aide de commandes AT.

Elle permet de :
- Vérifier la disponibilité du module GSM/4G.
- Vérifier l'accès au réseau et l'attachement GPRS.
- Effectuer une requête HTTP pour récupérer le fichier `fw_details.txt`.
- Extraire la taille (`SIZ`) et le CRC32 (`CRC`) du firmware distant.
- Comparer ces informations avec celles du firmware actuellement utilisé.

Lorsqu'une mise à jour est détectée, l'application déclenche un redémarrage du microcontrôleur avec `NVIC_SystemReset()`.

Le bootloader prend ensuite en charge le processus de mise à jour du firmware.

## Indication de fonctionnement

L'application utilise également des sorties GPIO pour piloter des LED et afficher visuellement l'application active lors des tests.

## Remarque

Le fichier `main.c` est présenté à titre de consultation du code source. Il dépend des bibliothèques STM32 HAL et des fichiers de configuration générés par STM32CubeIDE, qui ne sont pas inclus dans ce dossier.

Le code source du bootloader n'est pas publié dans ce dépôt.
