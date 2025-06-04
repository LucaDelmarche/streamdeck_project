import os

import psutil
from flask import Flask, jsonify, send_from_directory
from py3nvml import py3nvml as nvml

app = Flask(__name__)
nvml.nvmlInit()
handle = nvml.nvmlDeviceGetHandleByIndex(0)

# 📊 Route pour les métriques système
@app.route("/metrics")
def get_metrics():
    try:
        return jsonify({
            "cpu": int(psutil.cpu_percent(interval=0.2)),
            "gpu": int(nvml.nvmlDeviceGetUtilizationRates(handle).gpu),
            "ram": int(psutil.virtual_memory().percent),
            "disk": int(psutil.disk_usage("C:\\").percent),
            "temp": int(nvml.nvmlDeviceGetTemperature(handle, nvml.NVML_TEMPERATURE_GPU))
        })
    except Exception as e:
        return jsonify({"error": str(e)}), 500

# 🖼️ Route pour servir les fichiers .rgb
@app.route("/images/<path:filename>")
def serve_rgb(filename):
    return send_from_directory(os.path.join("images"), filename)

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=8080)
