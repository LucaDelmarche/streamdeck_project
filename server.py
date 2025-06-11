import json
import os
import subprocess

import psutil
from flask import Flask, jsonify, request, send_from_directory
from flask_cors import CORS
from py3nvml import py3nvml as nvml

app = Flask(__name__, static_folder="frontend", static_url_path="")
CORS(app)
# Init GPU
nvml.nvmlInit()
handle = nvml.nvmlDeviceGetHandleByIndex(0)

# Serve React index.html on root and any route
@app.route("/")
@app.route("/<path:path>")
def serve_react(path=""):
    if path != "" and os.path.exists(os.path.join(app.static_folder, path)):
        return send_from_directory(app.static_folder, path)
    else:
        return send_from_directory(app.static_folder, "index.html")

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

@app.after_request
def add_header(response):
    response.headers["Cache-Control"] = "no-store"
    response.headers["Connection"] = "close"
    return response

@app.route("/images/<path:filename>")
def serve_rgb(filename):
    return send_from_directory("images", filename)

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=8080)
