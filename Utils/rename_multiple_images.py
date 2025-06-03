import os

# === CONFIGURATION ===
folder = "./frames"            # Dossier contenant les fichiers
prefix = "frame_"              # Préfixe de base
extension = ".gif"             # Nouvelle extension forcée
padding = 3                    # Nombre de chiffres pour le compteur

# === Lister et trier les fichiers à renommer ===
files = sorted([f for f in os.listdir(folder) if os.path.isfile(os.path.join(folder, f))])
print(f"Fichiers trouvés : {len(files)}")

for idx, filename in enumerate(files):
    new_filename = f"{prefix}{str(idx).zfill(padding)}{extension}"
    src = os.path.join(folder, filename)
    dst = os.path.join(folder, new_filename)

    # Éviter les collisions si le nom cible existe déjà
    if os.path.exists(dst):
        print(f"[!] Le fichier {dst} existe déjà, il sera écrasé.")

    os.rename(src, dst)
    print(f"{filename} -> {new_filename}")
