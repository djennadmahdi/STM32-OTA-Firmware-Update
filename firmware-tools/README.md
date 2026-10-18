# Outil de calcul de la taille et du CRC32 du firmware

Cet outil développé en langage C permet de calculer automatiquement la taille et le CRC32 d'un firmware STM32 à partir du fichier binaire `Application.bin`.

Il génère ensuite un fichier `fw_details.txt` contenant les informations nécessaires à la vérification du firmware lors du processus de mise à jour OTA.

## Fonctionnalités

- Lecture du fichier binaire `Application.bin`.
- Calcul de la taille du firmware en octets.
- Calcul du CRC32 avec le polynôme `0x04C11DB7`.
- Génération automatique du fichier `fw_details.txt`.

## Compilation

Le programme peut être compilé sous Windows avec GCC :

```bash
gcc etx_ota_update_main.c -Wall -Wextra -O2 -o etx_ota_app.exe
```

## Exécution

Depuis un terminal PowerShell, exécuter :

```powershell
.\etx_ota_app.exe "C:\chemin\vers\Application.bin"
```

Remplacer le chemin indiqué par celui du fichier `Application.bin` généré par STM32CubeIDE.

## Fichier généré

Le programme crée automatiquement `fw_details.txt` dans le répertoire de travail.

Son contenu est au format suivant :

```text
SIZ=0xXXXXXXXX
CRC=0xXXXXXXXX
```

- **SIZ** : taille du firmware en octets, exprimée en hexadécimal.
- **CRC** : valeur CRC32 calculée à partir du firmware.

Ces deux informations permettent au système OTA de contrôler les caractéristiques du firmware téléchargé.

## Remarque

Cet outil s'exécute sur un ordinateur et ne nécessite pas de connexion à la carte STM32. Il sert à préparer les métadonnées du firmware avant leur publication avec le fichier binaire sur le serveur de mise à jour.
