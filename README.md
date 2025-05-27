# Stream Deck ESP32


# /!\Disclaimer : Projet en cours, il subira des changements c'est normal si tout ne fonctionne pas
Ce projet met en oeuvre un "stream deck" basé sur un ESP32 avec un écran tactile ILI9341 et plusieurs boutons physiques. L'ESP32 envoie des commandes textuelles au PC via la liaison série. Un script Python interprète ces commandes et exécute les raccourcis clavier correspondants.

## Contenu du dépôt

- `streamdeck.ino` : programme Arduino pour l'ESP32. Il gère les menus affichés sur l'écran tactile et l'envoi de commandes série lorsque l'utilisateur appuie sur un bouton ou touche l'écran.
- `streamdeck_listener.py` : script Python qui attend les messages de l'ESP32 sur le port série. Il utilise `pyautogui` pour déclencher des actions sur le PC (contrôle de Spotify, commandes VSCode, etc.).
- `launch_streamdeck.bat` : script Windows pour lancer facilement `streamdeck_listener.py` depuis le bon répertoire.

## Mise en route rapide

1. Compiler puis téléverser `streamdeck.ino` sur l'ESP32.
2. Connecter la carte au PC et noter le port série associé (par défaut `COM8` dans le script Python, à adapter).
3. Installer Python et la bibliothèque `pyautogui`.
4. Lancer `streamdeck_listener.py` (ou `launch_streamdeck.bat` sous Windows).

Une fois en fonctionnement, l'appui sur un bouton ou une zone tactile du stream deck déclenche la commande correspondante sur le PC.
