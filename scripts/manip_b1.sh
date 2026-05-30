#!/usr/bin/env bash
#
# Manipulation B.1 : NOPASSWD via /etc/sudoers.d/
# Cible : creation d'un fichier sudoers dedie.
# Test : `sudo whoami` ne demande plus de mot de passe.
#

set -euo pipefail
USERNAME="${SUDO_USER:-$USER}"
NOPASSWD_FILE="/etc/sudoers.d/nopasswd"

echo "=== Manipulation B.1 : sudoers NOPASSWD ==="
echo "Utilisateur cible : $USERNAME"
echo

# --- Etat initial ---
echo "[1/5] Verification de l'etat initial :"
sudo -k
echo "(la commande suivante doit demander un mot de passe)"
sudo whoami

# --- Application ---
echo
echo "[2/5] Creation de $NOPASSWD_FILE"
echo "$USERNAME ALL=(ALL) NOPASSWD: ALL" | sudo tee "$NOPASSWD_FILE" > /dev/null
sudo chmod 0440 "$NOPASSWD_FILE"

# --- Test ---
echo "[3/5] Test du contournement :"
sudo -k
echo "(la commande suivante NE doit PAS demander de mot de passe)"
sudo whoami

# --- Pause ---
echo
read -p "Appuyer sur [Entree] pour restaurer..." _

# --- Restauration ---
echo "[4/5] Suppression de $NOPASSWD_FILE"
sudo rm -f "$NOPASSWD_FILE"

# --- Verification ---
echo "[5/5] Verification (sudo doit redemander le mot de passe) :"
sudo -k
sudo whoami
