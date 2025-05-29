import struct

from PIL import Image

# Entrée BMP
input_file = "VSCode.bmp"
# Sortie RAW RGB565
output_file = "VSCode.rgb"

# Ouvrir, convertir, redimensionner si besoin
img = Image.open(input_file).convert("RGB")  # Assure RGB
img = img.resize((100, 75))  # Adapte à la taille de ton bouton

# Conversion en RGB565
with open(output_file, "wb") as f:
    for pixel in img.getdata():
        r, g, b = pixel
        rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
        f.write(struct.pack(">H", rgb565))  # Big endian
