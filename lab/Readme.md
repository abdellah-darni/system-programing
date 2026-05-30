# Chat Hub — Application de Chat Centralisée

Serveur unique (Chat Hub) en architecture client–serveur. Tous les messages
échangés entre les clients transitent obligatoirement par le serveur, qui les
relaie aux destinataires (diffusion ou message privé).

> Documentation détaillée (schéma d'architecture, description des messages) :
> voir [`DOCS.md`](DOCS.md). Jeu de tests : voir [`tests/SCENARIOS.md`](tests/SCENARIOS.md).

## Choix techniques

| Aspect            | Choix                                | Justification                                          |
|-------------------|--------------------------------------|--------------------------------------------------------|
| Langage           | C (C11)                              | Cohérent avec les TP précédents `sockets1`, `sockets2` |
| Protocole         | TCP/IP                               | Ordre FIFO garanti, détection de déconnexion native    |
| Concurrence       | `pthread` (un thread par client)     | Modèle simple, lisible, suffisant pour ≤ 64 clients    |
| Format wire       | texte, lignes terminées par `\n`     | Inspectable au `nc`, facile à étendre                  |
| Interface client  | CLI (ligne de commande)              | Pas de dépendance graphique                            |
| Journal optionnel | `--log <fichier>` côté serveur       | Trace horodatée des évènements (JOIN/MSG/PRIV/LEAVE)   |

## Organisation des fichiers

```
.
├── server.c            # serveur (accept loop + 1 thread/client + table partagée)
├── client.c            # client CLI (thread d'envoi + thread de réception)
├── common.h            # protocole, line_reader_t, send_line/send_all, constantes
├── Makefile            # build + cible `make check`
├── README.md           # ce fichier
├── DOCS.md             # documentation succincte (architecture + messages)
└── tests/
    ├── run_tests.sh    # suite de tests automatique
    └── SCENARIOS.md    # scénarios couverts + scénario de démonstration
```

## Dépendances et compilation

Nécessite uniquement `gcc` et la libc POSIX (Linux). Aucune bibliothèque tierce.
Le jeu de tests automatique requiert en plus `nc` (netcat).

```bash
make            # produit ./chat_serverd et ./chat_client
make check      # compile puis lance la suite de tests automatique
make clean
```

## Exécution

```bash
# Terminal serveur
./chat_serverd --port 5555
./chat_serverd --port 5555 --log chat.log   # avec historique horodaté

# Terminal client
./chat_client --server localhost --port 5555
```

À la connexion, le client demande un pseudo. Si le nom est déjà pris ou invalide
(caractères autorisés : alphanumériques, `_`, `-`, max 31 caractères), le
serveur répond `ERR …` et l'utilisateur peut réessayer.

## Commandes côté client

| Commande              | Effet                                                       |
|-----------------------|-------------------------------------------------------------|
| `<texte>`             | Diffuse `<texte>` à tous les autres clients                 |
| `/msg <pseudo> <txt>` | Message privé routé vers `<pseudo>` uniquement              |
| `/users`              | Demande la liste des participants connectés                 |
| `/quit`               | Déconnexion propre (notifiée aux autres)                    |

## Protocole de communication

Lignes ASCII terminées par `\n`. Le premier *token* est le verbe. La
spécification complète (tableaux + exemple de session) est dans
[`DOCS.md`](DOCS.md) ; résumé ci-dessous.

### Client → Serveur

| Message              | Sémantique                                              |
|----------------------|---------------------------------------------------------|
| `NICK <name>`        | Handshake. Doit être le premier message envoyé.         |
| `MSG <texte>`        | Diffusion à tous les autres clients.                    |
| `PRIV <name> <txt>`  | Message privé vers `<name>`.                            |
| `LIST`               | Demande la liste des participants.                      |
| `QUIT`               | Déconnexion explicite.                                  |

### Serveur → Client

| Message              | Sémantique                                              |
|----------------------|---------------------------------------------------------|
| `OK`                 | Pseudo accepté (réponse à `NICK`).                      |
| `ERR <raison>`       | Pseudo refusé ou commande invalide.                     |
| `MSG <from> <txt>`   | Diffusion reçue d'un autre client.                      |
| `PRIV <from> <txt>`  | Message privé reçu.                                     |
| `JOIN <name>`        | Notification d'arrivée.                                 |
| `LEAVE <name>`       | Notification de départ (propre ou brutale).             |
| `LIST <n1> <n2> …`   | Réponse à `LIST`.                                       |
| `SYS <texte>`        | Message système générique.                              |

## Garanties

* **Ordre FIFO** : assuré par TCP par socket, et chaque émission est
  effectuée par un thread unique.
* **Déconnexion brutale** : le `recv()` du thread client retourne 0/-1,
  ce qui déclenche la diffusion d'un `LEAVE` et la libération du slot.
* **Non-blocage du serveur** : la diffusion ne tient pas le verrou pendant
  les `send()`, et `SO_SNDTIMEO` (5 s) empêche un client gelé de bloquer les
  autres. La boucle `accept()` reste toujours disponible.
* **Unicité du pseudo** : vérifiée sous verrou avant insertion dans la
  table ; doublon → `ERR name already taken`.

## Difficultés rencontrées

* **Affichage CLI concurrent.** Les messages entrants doivent s'insérer
  proprement même quand l'utilisateur est en train de saisir une commande.
  La séquence `\r\033[K` (retour ligne + effacement) est imprimée avant
  chaque message reçu, puis le prompt `> ` est réaffiché.
* **Fragmentation TCP.** Le code n'assume jamais qu'un `recv()` renvoie une
  ligne complète : un `line_reader_t` (dans `common.h`) bufferise les
  octets et ne livre que des lignes complètes au code applicatif.
* **Slow client.** Les `send()` de diffusion sont effectués hors verrou et
  avec un timeout pour éviter qu'un client gelé fige le serveur.

## Jeu de tests

Une suite automatique rejoue chaque scénario via le protocole brut et vérifie
les réponses (18 assertions, code de retour `0`/`1`) :

```bash
make check          # ou : ./tests/run_tests.sh
```

Le détail des cas couverts et le scénario de démonstration manuel sont décrits
dans [`tests/SCENARIOS.md`](tests/SCENARIOS.md).

## Limites connues / extensions possibles

* **Handshake sans timeout de lecture.** Une connexion qui n'envoie jamais
  `NICK` occupe un thread indéfiniment. Un `SO_RCVTIMEO` sur la phase de
  handshake fermerait les connexions muettes (durcissement anti-slowloris).
* **Troncature des très longs messages.** Un client brut (`nc`) envoyant un
  `MSG` proche de 1024 octets verrait le texte tronqué côté serveur (le
  préfixe `MSG <pseudo> ` déborde du tampon). Le client fourni borne la
  saisie pour éviter ce cas.
* **Sécurité** : ajouter TLS via OpenSSL, ou un mot de passe par pseudo.
* **UDP** : le protocole étant texte ligne-à-ligne, un mode UDP est trivial à
  ajouter (un datagramme = une ligne).
* **Salons multiples** : remplacer la diffusion globale par un mapping
  pseudo → canal et router `MSG` au canal courant.
