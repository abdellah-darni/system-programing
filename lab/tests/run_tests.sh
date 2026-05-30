#!/usr/bin/env bash
#
# run_tests.sh — Jeu de tests automatique pour Chat Hub
#
# Pilote le serveur via le protocole texte brut (avec `nc`), sans passer par
# le client interactif, pour obtenir des assertions déterministes. Chaque
# scénario du sujet est rejoué et vérifié ; le script sort avec un code != 0
# si au moins un test échoue.
#
# Dépendances : bash, nc (netcat), le binaire ./chat_serverd compilé.
# Usage       : ./tests/run_tests.sh        (depuis la racine du projet)
#
set -u

# --- Configuration -----------------------------------------------------------
PORT="${PORT:-5599}"                 # port de test (surchageable : PORT=... )
SERVER="${SERVER:-./chat_serverd}"   # chemin du binaire serveur
STEP="${STEP:-0.4}"                  # délai entre les actions (synchronisation)
LOG="$(mktemp /tmp/chathub_test.XXXXXX.log)"
WORK="$(mktemp -d /tmp/chathub_test.XXXXXX)"

PASS=0
FAIL=0

# --- Couleurs (désactivées si sortie non-TTY) --------------------------------
if [ -t 1 ]; then G=$'\033[32m'; R=$'\033[31m'; B=$'\033[1m'; Z=$'\033[0m'
else            G=''; R=''; B=''; Z=''; fi

# --- Cadre d'assertions ------------------------------------------------------
# assert_has <fichier> <motif> <description>  : le motif DOIT apparaître
assert_has() {
    if grep -qF -- "$2" "$1"; then
        printf "  ${G}[PASS]${Z} %s\n" "$3"; PASS=$((PASS + 1))
    else
        printf "  ${R}[FAIL]${Z} %s\n" "$3"
        printf "         attendu (présent) : %s\n" "$2"
        printf "         obtenu           : %s\n" "$(tr '\n' '|' < "$1")"
        FAIL=$((FAIL + 1))
    fi
}
# assert_absent <fichier> <motif> <description> : le motif NE doit PAS apparaître
assert_absent() {
    if grep -qF -- "$2" "$1"; then
        printf "  ${R}[FAIL]${Z} %s\n" "$3"
        printf "         interdit mais présent : %s\n" "$2"
        FAIL=$((FAIL + 1))
    else
        printf "  ${G}[PASS]${Z} %s\n" "$3"; PASS=$((PASS + 1))
    fi
}

# --- Gestion du serveur ------------------------------------------------------
SRV_PID=""
start_server() {
    : > "$LOG"
    "$SERVER" --port "$PORT" --log "$LOG" >/dev/null 2>&1 &
    SRV_PID=$!
    sleep 0.3
    if ! kill -0 "$SRV_PID" 2>/dev/null; then
        echo "${R}Impossible de démarrer le serveur (port $PORT déjà pris ?)${Z}"
        exit 2
    fi
}
stop_server() {
    [ -n "$SRV_PID" ] && kill "$SRV_PID" 2>/dev/null
    wait "$SRV_PID" 2>/dev/null
    SRV_PID=""
}

cleanup() { stop_server; rm -rf "$WORK" "$LOG"; }
trap cleanup EXIT

# --- Clients « one-shot » ----------------------------------------------------
# oneshot <fichier_sortie> <ligne...> : envoie les lignes puis ferme,
# capture la réponse du serveur dans <fichier_sortie>.
oneshot() {
    local out="$1"; shift
    printf '%s\n' "$@" | nc -q1 127.0.0.1 "$PORT" > "$out" 2>&1
}

# --- Clients persistants (connexion maintenue ouverte) -----------------------
# Permet les scénarios concurrents (diffusion, privé, JOIN/LEAVE). Chaque
# ouverture utilise des chemins uniques pour éviter toute collision entre tests.
declare -A C_FD C_OUT C_PID
SEQ=0
client_open() {                       # client_open <id>
    local id="$1" fd
    SEQ=$((SEQ + 1))
    local fifo="$WORK/in_$SEQ"
    mkfifo "$fifo"
    C_OUT[$id]="$WORK/out_$SEQ"
    nc -q1 127.0.0.1 "$PORT" < "$fifo" > "${C_OUT[$id]}" 2>&1 &
    C_PID[$id]=$!
    exec {fd}>"$fifo"                 # maintient l'entrée ouverte
    C_FD[$id]=$fd
}
client_send() { printf '%s\n' "$2" >&"${C_FD[$1]}"; sleep "$STEP"; }
client_close(){                       # ferme l'entrée (EOF) puis arrête nc
    local fd="${C_FD[$1]}"
    eval "exec ${fd}>&-"
    kill "${C_PID[$1]}" 2>/dev/null
    wait "${C_PID[$1]}" 2>/dev/null
}
client_out()  { echo "${C_OUT[$1]}"; }

# =============================================================================
echo "${B}== Jeu de tests Chat Hub ==${Z}"
echo "port=$PORT  serveur=$SERVER"

if [ ! -x "$SERVER" ]; then
    echo "${R}Binaire $SERVER introuvable. Lancez 'make' d'abord.${Z}"
    exit 2
fi

# -----------------------------------------------------------------------------
echo
echo "${B}1) Handshake et validation du pseudo${Z}"
start_server
oneshot "$WORK/t1" "NICK Kamal" "QUIT"
assert_has "$WORK/t1" "OK" "pseudo valide accepté (OK)"

oneshot "$WORK/t2" "NICK bad name" "QUIT"
assert_has "$WORK/t2" "ERR invalid name" "pseudo avec espace refusé"

oneshot "$WORK/t3" "MSG salut" "QUIT"
assert_has "$WORK/t3" "ERR expected NICK first" "NICK obligatoire avant tout"
stop_server

# -----------------------------------------------------------------------------
echo
echo "${B}2) Unicité du pseudo${Z}"
start_server
client_open A
client_send A "NICK Kamal"
oneshot "$WORK/dup" "NICK Kamal" "QUIT"     # second client, même pseudo
assert_has "$WORK/dup" "ERR name already taken" "doublon de pseudo refusé"
client_close A
stop_server

# -----------------------------------------------------------------------------
echo
echo "${B}3) Diffusion (broadcast) et exclusion de l'émetteur${Z}"
start_server
client_open A; client_send A "NICK Kamal"
client_open B; client_send B "NICK Said"
client_send B "MSG Bonjour tout le monde"
client_send A "QUIT"; client_send B "QUIT"
client_close A; client_close B
assert_has    "$(client_out A)" "MSG Said Bonjour tout le monde" "Kamal reçoit le message de Said"
assert_absent "$(client_out B)" "MSG Said Bonjour"               "Said ne reçoit pas son propre message"
assert_has    "$(client_out A)" "JOIN Said"                      "Kamal est notifié de l'arrivée de Said"
stop_server

# -----------------------------------------------------------------------------
echo
echo "${B}4) Message privé (routage ciblé)${Z}"
start_server
client_open A; client_send A "NICK Kamal"
client_open B; client_send B "NICK Said"
client_open C; client_send C "NICK Omar"
client_send B "PRIV Kamal Coucou en prive"
client_send A "QUIT"; client_send B "QUIT"; client_send C "QUIT"
client_close A; client_close B; client_close C
assert_has    "$(client_out A)" "PRIV Said Coucou en prive" "Kamal reçoit le message privé"
assert_absent "$(client_out C)" "Coucou en prive"           "Omar (tiers) ne voit pas le privé"
stop_server

# -----------------------------------------------------------------------------
echo
echo "${B}5) Cas d'erreur PRIV${Z}"
start_server
oneshot "$WORK/p1" "NICK Kamal" "PRIV Fantome salut" "QUIT"
assert_has "$WORK/p1" "ERR no such user: Fantome" "PRIV vers inconnu → erreur"
oneshot "$WORK/p2" "NICK Kamal" "PRIV Kamal a moi-meme" "QUIT"
assert_has "$WORK/p2" "ERR cannot PRIV yourself" "PRIV vers soi-même → erreur"
stop_server

# -----------------------------------------------------------------------------
echo
echo "${B}6) Liste des participants (LIST)${Z}"
start_server
client_open A; client_send A "NICK Kamal"
client_open B; client_send B "NICK Said"
client_send A "LIST"
client_send A "QUIT"; client_send B "QUIT"
client_close A; client_close B
assert_has "$(client_out A)" "LIST Kamal Said" "LIST renvoie les connectés"
stop_server

# -----------------------------------------------------------------------------
echo
echo "${B}7) Déconnexion propre et brutale (LEAVE)${Z}"
start_server
client_open A; client_send A "NICK Kamal"
client_open B; client_send B "NICK Said"
client_send B "QUIT"                       # départ propre de Said
sleep "$STEP"
client_close B
client_send A "QUIT"; client_close A
assert_has "$(client_out A)" "LEAVE Said" "Kamal est notifié du départ de Said"
# Le LEAVE figure aussi dans le journal horodaté
assert_has "$LOG" "[LEAVE] Said" "départ tracé dans le journal"
stop_server

# -----------------------------------------------------------------------------
echo
echo "${B}8) Commande inconnue${Z}"
start_server
oneshot "$WORK/u1" "NICK Kamal" "FOOBAR test" "QUIT"
assert_has "$WORK/u1" "ERR unknown command" "verbe inconnu → erreur"
stop_server

# -----------------------------------------------------------------------------
echo
echo "${B}9) Journalisation horodatée (--log)${Z}"
start_server
client_open A; client_send A "NICK Kamal"
client_send A "MSG ligne de journal"
client_send A "QUIT"; client_close A
sleep "$STEP"
assert_has "$LOG" "[JOIN] Kamal"              "JOIN journalisé"
assert_has "$LOG" "[MSG] Kamal: ligne de journal" "MSG journalisé"
# Vérifie la présence d'un horodatage AAAA-MM-JJ HH:MM:SS en tête de ligne
if grep -qE '^[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2} ' "$LOG"; then
    printf "  ${G}[PASS]${Z} %s\n" "lignes de journal horodatées"; PASS=$((PASS + 1))
else
    printf "  ${R}[FAIL]${Z} %s\n" "lignes de journal horodatées"; FAIL=$((FAIL + 1))
fi
stop_server

# =============================================================================
echo
echo "${B}== Bilan ==${Z}"
printf "  ${G}%d réussis${Z}, ${R}%d échoués${Z}\n" "$PASS" "$FAIL"
[ "$FAIL" -eq 0 ] && echo "${G}${B}Tous les tests passent.${Z}" || echo "${R}${B}Des tests ont échoué.${Z}"
exit "$([ "$FAIL" -eq 0 ] && echo 0 || echo 1)"
