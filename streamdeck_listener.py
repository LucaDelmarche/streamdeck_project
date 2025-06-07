import os
import subprocess
import threading
import time

import keyboard
import pyautogui
import serial
from dotenv import load_dotenv

auto_clicking = False
auto_clicking_thread = None

def auto_click_loop():
    while auto_clicking:
        pyautogui.click()
        time.sleep(2)

def toggle_auto_click():
    global auto_clicking, auto_click_thread
    if not auto_clicking:
        auto_clicking = True
        auto_click_thread = threading.Thread(target=auto_click_loop, daemon=True)
        auto_click_thread.start()
    else:
        auto_clicking = False
        
# .env loading
load_dotenv(override=True)

# Taking login fields from environment variables
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

# Long press for password, short press for email
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

#Those ids can be found in the URL of the game page on Steam
# Exemple : https://store.steampowered.com/app/359550/Rainbow_Six_Siege/
steam_games = {
    "5_0": "359550", #Rainbow Six Siege
    "5_4": "3058630", #Assetto Corsa Evolution
}

exe_games = {
    "5_3": "", #Content manager
    "5_1": "", #F1 24
    "5_2": "", #Jedi survivor
    "5_5": "C:\\Users\\lucad\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs\\Lunar Client.lnk", #Lunar client
}
vs_commands = {
    "1_0": "code --folder-uri vscode-remote://wsl+ubuntu/mnt/c/Rootly/git/qsbridge-bk",
    "1_1": "code C:\\Rootly\\git\\qsbridge-ft-revamp",
}
# System commands
# These commands are executed using the keyboard library to simulate key presses
commandes = {
    "0_0": lambda: keyboard.send("previous track"),
    "0_1": lambda: keyboard.send("play/pause media"),
    "0_2": lambda: keyboard.send("next track"),
    "0_3": lambda: keyboard.send("volume up"),
    "0_4": lambda: keyboard.send("volume down"),
    "0_5": lambda: keyboard.send("volume mute"),
    "2_0": lambda: toggle_auto_click(),
    "1_2": lambda: keyboard.send("f5"),
    "1_3": lambda: keyboard.send("ctrl+`"),
    "1_4": lambda: keyboard.send("ctrl+f"),
}

# Port configuration
ser = serial.Serial('COM8', 115200, timeout=1) # COM8 is the port where the ESP32 is connected, adjust as needed
print("En attente de commandes ESP32...")

# Main loop to listen for commands from the ESP32
while True:
    try:
        line = ser.readline().decode('utf-8', errors='ignore').strip()
        if not line:
            continue

        print("Commande reçue :", line)

        # Check if the command is a long press (ends with "_long")
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
        # Steam games
        elif key in steam_games:
            game_id = steam_games[key]
            subprocess.run(['start', 'steam://rungameid/' + game_id], shell=True)
        
        elif key in vs_commands:
            command = vs_commands[key]
            subprocess.run(command, shell=True)
        elif key in exe_games:
            exe_path = exe_games[key]
            if exe_path:
                subprocess.run(f'start "" "{exe_path}"', shell=True)
            else:
                print(f"[⚠️] Chemin manquant pour {key}")
        # Handle system commands
        elif key in commandes:
            commandes[key]()
        else:
            print(f"Commande inconnue : {line}")

    except KeyboardInterrupt:
        print("Arrêt par l'utilisateur.")
        break
    except Exception as e:
        print("Erreur :", e)
