import os
import struct

from PIL import Image

# === Paramètres ===
input_folder = "./images/sous_menu/1"        # Dossier où sont les .gif
output_folder = "./images/sous_menu/1"       # Dossier de sortie
output_size = (320, 240)           # Taille cible (ex: plein écran)

# Créer le dossier de sortie s'il n'existe pas
os.makedirs(output_folder, exist_ok=True)

# Fonction de conversion
def convert_image_to_rgb565(input_path, output_path, size):
    img = Image.open(input_path).convert("RGB")
    img = img.resize(size)

    with open(output_path, "wb") as f:
        for pixel in img.getdata():
            r, g, b = pixel
            rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
            f.write(struct.pack(">H", rgb565))  # Big endian

# Traitement de tous les GIF
for filename in os.listdir(input_folder):
    if filename.lower().endswith(".gif"):
        name = os.path.splitext(filename)[0]
        input_path = os.path.join(input_folder, filename)
        output_path = os.path.join(output_folder, name + ".rgb")
        print(f"Conversion : {filename} -> {name}.rgb")
        convert_image_to_rgb565(input_path, output_path, output_size)

print("Conversion terminee.")

# import os

# # Dossier à nettoyer
# target_folder = "./frames"  # Remplace par ton dossier

# # Suppression des .gif
# deleted_count = 0
# for filename in os.listdir(target_folder):
#     if filename.lower().endswith(".gif"):
#         file_path = os.path.join(target_folder, filename)
#         os.remove(file_path)
#         deleted_count += 1
#         print(f" Supprime : {filename}")

# print(f"\n{deleted_count} fichier(s) .gif supprime(s).")
