#!/usr/bin/env bash
#
# Manipulation B.2 : Modification de /etc/pam.d/sudo
# Cible : insertion de pam_permit.so dans la pile auth de sudo.
# Test : `sudo whoami` ne valide plus le mot de passe.
#

set -euo pipefail
PAM_FILE="/etc/pam.d/sudo"
BAK="${PAM_FILE}.bak"

echo "=== Manipulation B.2 : PAM sudo ==="
echo

# --- Sauvegarde ---
echo "[1/5] Sauvegarde vers $BAK"
sudo cp "$PAM_FILE" "$BAK"

# --- Modification ---
echo "[2/5] Insertion de pam_permit.so en tete de la pile auth"
sudo sed -i \
    -e '1a auth       sufficient   pam_permit.so' \
    -e 's|^auth       include      system-auth|#&|' \
    "$PAM_FILE"

echo "[3/5] Contenu apres modification :"
echo "----"
sudo cat "$PAM_FILE"
echo "----"

# --- Test ---
echo
echo ">>> Test : sudo -k && sudo whoami doit accepter n'importe quel mot de passe."
echo
read -p "Appuyer sur [Entree] une fois le test effectue, pour restaurer..." _

# --- Restauration ---
echo "[4/5] Restauration depuis $BAK"
sudo cp "$BAK" "$PAM_FILE"

# --- Verification ---
echo "[5/5] Verification :"
sudo diff "$PAM_FILE" "$BAK" && echo "OK : fichier restaure a l'identique."
sudo -k
sudo whoami
