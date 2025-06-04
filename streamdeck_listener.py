import keyboard
import serial

ser = serial.Serial('COM8', 115200, timeout=1)  # Timeout en secondes
print("En attente de commandes ESP32...")

while True:
    line = ser.readline().decode('utf-8', errors='ignore').strip()
    print("Commande reçue :", line)

    if line == "0_0":
        keyboard.send("previous track")
    elif line == "0_1":
        keyboard.send("play/pause media")
    elif line == "0_2":
        keyboard.send("next track")
    elif line == "0_3":
        keyboard.send("volume up")
    elif line == "0_4":
        keyboard.send("volume down")
    elif line == "0_5":
        keyboard.send("volume mute")

    elif line == "vscode_run":
        keyboard.send("f5")
    elif line == "vscode_terminal":
        keyboard.send("ctrl+`")
    elif line == "vscode_find":
        keyboard.send("ctrl+f")
    elif line == "vscode_comment":
        keyboard.send("ctrl+k, ctrl+c")  # enchaîne les deux
