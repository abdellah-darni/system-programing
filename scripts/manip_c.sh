#!/usr/bin/env bash
#
# Manipulation C : Desactivation globale via system-auth ET password-auth.
# Cible : toutes les piles PAM (login, sudo, su, sshd, cockpit, ...).
# Test : su et sudo acceptent indifferemment n'importe quel mot de passe.
#
# NOTE : sur Fedora, ces deux fichiers sont des liens symboliques vers
# /etc/authselect/. cp -L est utilise pour suivre les liens et capturer
# le contenu effectif.
#

set -euo pipefail
SYSTEM_AUTH="/etc/pam.d/system-auth"
PASSWORD_AUTH="/etc/pam.d/password-auth"
SYSTEM_BAK="${SYSTEM_AUTH}.bak"
PASSWORD_BAK="${PASSWORD_AUTH}.bak"

echo "=== Manipulation C : desactivation globale ==="
echo

# --- Sauvegarde (avec -L pour suivre les liens symboliques) ---
echo "[1/5] Sauvegarde de system-auth et password-auth"
sudo cp -L "$SYSTEM_AUTH"   "$SYSTEM_BAK"
sudo cp -L "$PASSWORD_AUTH" "$PASSWORD_BAK"

# --- Modification ---
echo "[2/5] Insertion de pam_permit.so en tete des deux fichiers"
for f in "$SYSTEM_AUTH" "$PASSWORD_AUTH"; do
    sudo sed -i '/^# See authselect/a auth        sufficient    pam_permit.so' "$f"
done

echo "[3/5] Apercu du resultat :"
echo "----"
sudo grep -A1 "^# See authselect" "$SYSTEM_AUTH" | head -5
echo "----"

# --- Test ---
echo
echo ">>> Test : 'sudo -k && sudo whoami' et 'su -' acceptent tout mot de passe."
echo
read -p "Appuyer sur [Entree] une fois le test effectue, pour restaurer..." _

# --- Restauration ---
echo "[4/5] Restauration depuis les sauvegardes"
sudo cp "$SYSTEM_BAK"   "$SYSTEM_AUTH"
sudo cp "$PASSWORD_BAK" "$PASSWORD_AUTH"

# --- Verification ---
echo "[5/5] Verification (les deux diffs doivent etre vides) :"
sudo diff "$SYSTEM_AUTH" "$SYSTEM_BAK"     && echo "OK : system-auth restaure."
sudo diff "$PASSWORD_AUTH" "$PASSWORD_BAK" && echo "OK : password-auth restaure."
sudo -k
sudo whoami
