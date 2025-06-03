import keyboard
import serial

ser = serial.Serial('COM8', 115200, timeout=1)  # Timeout en secondes
print("En attente de commandes ESP32...")

while True:
    print("En attente de données...")
    line = ser.readline().decode('utf-8', errors='ignore').strip()
    print("Commande reçue :", line)

    if line == "0":
        keyboard.send("play/pause media")
    elif line == "1":
        keyboard.send("next track")
    elif line == "2":
        keyboard.send("previous track")
    elif line == "vscode_build":
        keyboard.send("f7")
    elif line == "vscode_run":
        keyboard.send("f5")
    elif line == "vscode_terminal":
        keyboard.send("ctrl+`")
    elif line == "vscode_find":
        keyboard.send("ctrl+f")
    elif line == "vscode_comment":
        keyboard.send("ctrl+k, ctrl+c")  # enchaîne les deux
