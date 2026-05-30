#!/usr/bin/env bash
#
# Manipulation A : Desactivation du mot de passe pour la connexion TTY (login)
# Cible : /etc/pam.d/login
# Test : connexion sur la console Proxmox avec n'importe quel mot de passe.
#

set -euo pipefail
PAM_FILE="/etc/pam.d/login"
BAK="${PAM_FILE}.bak"

echo "=== Manipulation A : login (TTY) ==="
echo "Fichier cible : $PAM_FILE"
echo

# --- Sauvegarde ---
echo "[1/5] Sauvegarde vers $BAK"
sudo cp "$PAM_FILE" "$BAK"

# --- Modification ---
echo "[2/5] Insertion de pam_permit.so et commentaire de la ligne d'origine"
sudo sed -i \
    -e '1a auth       sufficient   pam_permit.so' \
    -e 's|^auth       substack     system-auth|#&|' \
    "$PAM_FILE"

echo "[3/5] Verification du contenu modifie :"
echo "----"
sudo cat "$PAM_FILE" | head -5
echo "----"

# --- Pause pour le test ---
echo
echo ">>> Test : ouvrir une console TTY (Proxmox) et se connecter avec un mot"
echo ">>> de passe quelconque. La connexion doit reussir."
echo
read -p "Appuyer sur [Entree] une fois le test effectue, pour restaurer..." _

# --- Restauration ---
echo "[4/5] Restauration depuis $BAK"
sudo cp "$BAK" "$PAM_FILE"

# --- Verification ---
echo "[5/5] Verification (diff doit etre vide) :"
sudo diff "$PAM_FILE" "$BAK" && echo "OK : fichier restaure a l'identique."
