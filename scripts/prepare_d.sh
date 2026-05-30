#!/usr/bin/env bash
#
# Preparation pour la Manipulation D : clone, modification, compilation
# de linux-pam. A executer une fois avant manip_d.sh.
#

set -euo pipefail
REPO_DIR="$HOME/linux-pam"
PAM_VERSION="v1.7.1"   # ajuster selon `rpm -q pam`

echo "=== Preparation Manipulation D ==="

# --- Dependances ---
echo "[1/5] Installation des dependances de compilation"
sudo dnf install -y autoconf automake libtool gettext-devel libdb-devel \
    pam-devel docbook-style-xsl libxslt openssl-devel libxcrypt-devel \
    flex bison meson ninja-build git

# --- Clone ---
echo "[2/5] Clone de linux-pam ($PAM_VERSION)"
if [[ ! -d "$REPO_DIR" ]]; then
    git clone https://github.com/linux-pam/linux-pam.git "$REPO_DIR"
fi
cd "$REPO_DIR"
git checkout "$PAM_VERSION"

# --- Modification du code source ---
echo "[3/5] Modification de _unix_verify_password dans support.c"
TARGET="modules/pam_unix/support.c"
# On insere apres la ligne 'int _unix_verify_password(...)' suivie de l'accolade
# Sentinel sur la signature pour rester robuste.
if ! grep -q "ATELIER : bypass de la verification" "$TARGET"; then
    sudo sed -i '/^int _unix_verify_password/,/^{/{
        /^{/a\
\	/* ===== ATELIER : bypass de la verification ===== */\
\	(void)pamh; (void)name; (void)p; (void)ctrl;\
\	return PAM_SUCCESS;
    }' "$TARGET"
fi

grep -n "ATELIER" "$TARGET" || { echo "ERREUR : insertion echouee"; exit 1; }

# --- Compilation ---
echo "[4/5] Compilation avec meson"
meson setup builddir --prefix=/usr --libdir=/usr/lib64 || true
meson compile -C builddir

# --- Verification ---
BUILT="$REPO_DIR/builddir/modules/pam_unix/pam_unix.so"
if [[ -f "$BUILT" ]]; then
    echo "[5/5] OK : module compile a $BUILT"
    ls -la "$BUILT"
else
    echo "ERREUR : compilation echouee"
    exit 1
fi
