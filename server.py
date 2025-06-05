import json
import os
import subprocess

import psutil
from flask import Flask, jsonify, request, send_from_directory
from py3nvml import py3nvml as nvml

app = Flask(__name__)

# Initialisation GPU
nvml.nvmlInit()
handle = nvml.nvmlDeviceGetHandleByIndex(0)

# Lire proprement le token BW
def get_bw_session_token():
    try:
        with open("bw-session.txt", "r") as f:
            return f.read().strip()
    except Exception as e:
        print("[ERREUR] Impossible de lire bw-session.txt :", e)
        return None

@app.route("/metrics")
def get_metrics():
    try:
        disk_usages = []
        for partition in psutil.disk_partitions(all=False):
            try:
                usage = psutil.disk_usage(partition.mountpoint).percent
                disk_usages.append(usage)
            except (PermissionError, FileNotFoundError):
                continue
        avg_disk = int(sum(disk_usages) / len(disk_usages)) if disk_usages else 0

        return jsonify({
            "cpu": int(psutil.cpu_percent(interval=0.2)),
            "gpu": int(nvml.nvmlDeviceGetUtilizationRates(handle).gpu),
            "ram": int(psutil.virtual_memory().percent),
            "disk": avg_disk,
            "temp": int(nvml.nvmlDeviceGetTemperature(handle, nvml.NVML_TEMPERATURE_GPU))
        })

    except Exception as e:
        return jsonify({"error": str(e)}), 500

@app.route("/get_password")
def get_pw():
    site = request.args.get("site")
    if not site:
        return jsonify({"error": "Paramètre ?site manquant"}), 400

    token = get_bw_session_token()
    if not token:
        return jsonify({"error": "Session Bitwarden non trouvée"}), 500

    try:
        result = subprocess.run(
            ["C:\\Users\\lucad\\AppData\\Roaming\\npm\\bw.cmd", "get", "item", site, "--session", token],
            capture_output=True, text=True, timeout=5
        )
    except subprocess.TimeoutExpired:
        return jsonify({"error": "Timeout subprocess"}), 504
    except Exception as e:
        return jsonify({"error": f"Erreur subprocess: {str(e)}"}), 500

    if result.returncode != 0:
        return jsonify({"error": "Commande bw échouée", "stderr": result.stderr}), 500

    try:
        data = json.loads(result.stdout)
        login = data["login"]["username"]
        password = data["login"]["password"]
        return jsonify({"login": login, "password": password})
    except Exception as e:
        return jsonify({"error": f"Erreur parsing JSON: {str(e)}"}), 500

# Fichiers .rgb
@app.route("/images/<path:filename>")
def serve_rgb(filename):
    return send_from_directory("images", filename)

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=8080)
