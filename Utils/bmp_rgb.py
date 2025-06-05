import struct

from PIL import Image

# === Paramètres ===
input_file = "5.png"         # Nom du fichier BMP source
output_file = "5.rgb"       # Nom du fichier de sortie
output_size = (100, 75)                # Taille cible (largeur, hauteur)

# === Conversion ===
img = Image.open(input_file).convert("RGB")     # Force en RGB
img = img.resize(output_size)                   # Redimensionne

with open(output_file, "wb") as f:
    for pixel in img.getdata():
        r, g, b = pixel
        # Conversion RGB888 → RGB565
        rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
        f.write(struct.pack(">H", rgb565))      # Big endian : 2 octets par pixel

print("Fichier RGB565 genere :", output_file)
