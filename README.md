# ESP32 Stream Deck

This project implements a DIY stream deck using an ESP32 with an ILI9341 touch display and physical buttons. The board communicates with a host computer over serial to execute keyboard shortcuts and can display system information fetched from a local Flask server.

## Repository layout

- **`streamdeck/streamdeck.ino`** – Arduino sketch for the ESP32. It shows menus on the display, connects to Wi‑Fi, fetches images and system metrics from the server and sends commands over serial.
- **`server.py`** – Flask application serving a small React frontend, RGB images and system metrics (CPU, GPU, RAM, disk and temperature).
- **`streamdeck_listener.py`** – Python script listening for commands from the ESP32 and triggering keyboard actions on the PC via `pyautogui`.
- **`images/`** – RGB565 images displayed on the device (`menu` and `sous_menu` folders).
- **`Utils/`** – Utility scripts for converting images.
- **`frontend/`** – Pre‑built React assets served by `server.py`.

## Installation

1. **Clone the repository**
   ```bash
   git clone <repo>
   cd streamdeck_project
   ```
2. **Install Python dependencies**
   ```bash
   pip install -r requirements.txt
   ```
3. **Create a `.env` file** with any credentials used by `streamdeck_listener.py` (see variable names in the script).
4. **Upload `streamdeck/streamdeck.ino` to your ESP32** using the Arduino IDE. On first boot the board exposes a Wi‑Fi access point `ESP_Config` allowing you to enter your network credentials and the server address.
5. **Start the server and the listener**
   ```bash
   # In one terminal
   python server.py

   # In another terminal
   python streamdeck_listener.py
   ```
   On Windows you can use `launch_server.bat` and `launch_streamdeck.bat`.

## Usage

Once everything is running:

- The ESP32 fetches menu images from `server.py` and sends commands through the serial port when a button is pressed or held.
- `streamdeck_listener.py` interprets these commands and performs keyboard shortcuts or launches applications.
- Pressing the bottom‑right button toggles a system monitor view on the ESP32 using metrics provided by the Flask server.

Images for the menus are stored in `images/menu` and `images/sous_menu/<menu_number>`; replacing these files lets you customise the interface. Keyboard actions and program launches can be modified directly in `streamdeck_listener.py`.

## Component details & customisation

### `streamdeck_listener.py`

Listens on the serial port for commands coming from the ESP32. Each button ID is
looked up in a few dictionaries:

- `commandes` – media keys and other system shortcuts
- `steam_games` – maps a button to a Steam game ID (e.g. `359550` for Rainbow
  Six Siege)
- `exe_games` – paths to Windows executables to launch
- `vs_commands` – Visual Studio Code commands or folders
- `login_fields` – credentials loaded from environment variables

You can personalise the deck by editing these dictionaries. For instance, change
the value in `steam_games` to start any game from your library.

### `server.py`

Flask server providing the React frontend, RGB images and system metrics. The
host and port used when running the app can be tweaked at the bottom of this
file.

### `streamdeck/streamdeck.ino`

Firmware for the ESP32 that draws the menus, handles touch/buttons and sends
events over serial. On first boot the board exposes an `ESP_Config` Wi‑Fi access
point which lets you enter your network credentials and the server address.

---
This repository is a work in progress and may change over time.
