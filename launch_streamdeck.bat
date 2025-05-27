###Work in progress
###a terme le script se lancera en arrière plan mais pendant le développement je l'affiche pour débugger

@echo on
echo Lancement du script Stream Deck...
cd "C:\Users\lucad\OneDrive\Documents\Arduino\streamdeck" ## à changer avec votre chemin vers le fichier python
python streamdeck_listener.py
pause
