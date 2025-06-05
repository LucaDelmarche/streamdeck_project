import os
import time

import keyboard
import serial
from dotenv import load_dotenv

# Chargement du .env
load_dotenv(override=True)

# Récupération des identifiants depuis .env
login_fields = {
    "3_0": os.getenv("GITHUB"),
    "3_1": os.getenv("EA"),
    "3_2": os.getenv("UBISOFT"),
    "3_3": os.getenv("MICROSOFT"),
    "3_4": os.getenv("GMAIL1"),
    "3_5": os.getenv("GMAIL2"),
    "3_6": os.getenv("PAYPAL"),
    "4_0": os.getenv("STEAM"),
    "4_1": os.getenv("ROCKSTAR"),
    "4_2": os.getenv("APPLE"),
    "4_3": os.getenv("RIOT"),
    "4_4": os.getenv("AMAZON"),
    "4_5": os.getenv("BITWARDEN"),
}

# Fonctions pour remplissage court ou long
def fill_email_only(data):
    email = data.replace("'", "").split(",", 1)[0].strip()
    time.sleep(0.5)
    keyboard.write(email)
def fill_password(data):
    password = data.replace("'", "").split(",", 1)[1].strip()
    time.sleep(0.5)
    keyboard.write(password)
    time.sleep(0.5)
    keyboard.send("enter")

# Commandes système indépendantes
commandes = {
    "0_0": lambda: keyboard.send("previous track"),
    "0_1": lambda: keyboard.send("play/pause media"),
    "0_2": lambda: keyboard.send("next track"),
    "0_3": lambda: keyboard.send("volume up"),
    "0_4": lambda: keyboard.send("volume down"),
    "0_5": lambda: keyboard.send("volume mute"),
    "vscode_run": lambda: keyboard.send("f5"),
    "vscode_terminal": lambda: keyboard.send("ctrl+`"),
    "vscode_find": lambda: keyboard.send("ctrl+f"),
}

# Ouverture du port série
ser = serial.Serial('COM8', 115200, timeout=1)
print("En attente de commandes ESP32...")

# Boucle principale
while True:
    try:
        line = ser.readline().decode('utf-8', errors='ignore').strip()
        if not line:
            continue

        print("Commande reçue :", line)

        # Détection clic long
        key = line.replace("_long", "")
        is_long = line.endswith("_long")

        # Login fields
        if key in login_fields:
            creds = login_fields[key]
            if not creds:
                print(f"[⚠️] Données manquantes pour {key}")
                continue
            if is_long:
                fill_password(creds)
            else:
                fill_email_only(creds)

        # Commandes classiques
        elif key in commandes:
            commandes[key]()
        else:
            print(f"Commande inconnue : {line}")

    except KeyboardInterrupt:
        print("Arrêt par l'utilisateur.")
        break
    except Exception as e:
        print("Erreur :", e)
