import pyautogui
import serial

ser = serial.Serial('COM3', 115200)
print("En attente de commandes ESP32...")

while True:
    line = ser.readline().decode('utf-8', errors='ignore').strip()

    print("Commande reçue :", line)

    if line == "16":
        pyautogui.press("space")
    elif line == "spotify_next":
        pyautogui.hotkey("ctrl", "right")
    elif line == "spotify_prev":
        pyautogui.hotkey("ctrl", "left")
    elif line == "vscode_build":
        pyautogui.press("f7")
    elif line == "vscode_run":
        pyautogui.press("f5")
    elif line == "vscode_terminal":
        pyautogui.hotkey("ctrl", "`")
    elif line == "vscode_find":
        pyautogui.hotkey("ctrl", "f")
    elif line == "vscode_comment":
        pyautogui.hotkey("ctrl", "k")
        pyautogui.hotkey("ctrl", "c")
