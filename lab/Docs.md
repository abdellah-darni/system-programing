# Documentation — Chat Hub

Documentation succincte : schéma d'architecture et description des messages
échangés. Pour l'installation, l'exécution et les choix techniques, voir
[`README.md`](README.md).

## 1. Schéma d'architecture

Topologie en étoile : un serveur central, N clients. Aucun trafic client↔client
direct — tout passe par le Chat Hub.

```
                          CHAT HUB  (serveur TCP)
       ┌──────────────────────────────────────────────────────────┐
       │                                                            │
       │   thread principal                                         │
       │   ┌────────────┐                                           │
       │   │  accept()  │  boucle d'acceptation des connexions      │
       │   └─────┬──────┘                                           │
       │         │  pthread_create  (1 thread détaché par client)   │
       │         ▼                                                  │
       │   ┌──────────────┐  ┌──────────────┐  ┌──────────────┐     │
       │   │ client_thread│  │ client_thread│  │ client_thread│ ... │
       │   └──────┬───────┘  └──────┬───────┘  └──────┬───────┘     │
       │          │                 │                 │             │
       │          └────────┬────────┴────────┬────────┘             │
       │                   ▼                 ▼                       │
       │        ┌─────────────────────────────────────┐            │
       │        │  clients[MAX_CLIENTS]   (table)       │            │
       │        │  { fd, pseudo, joined_at, in_use }    │            │
       │        │  protégée par pthread_mutex_t         │            │
       │        └─────────────────────────────────────┘            │
       │                                                            │
       │        log_fp  (journal horodaté, mutex dédié)  [--log]    │
       └───────────┬─────────────────────────────┬──────────────────┘
                   │ TCP / lignes \n             │ TCP / lignes \n
                   ▼                             ▼
           ┌────────────────┐            ┌────────────────┐
           │  CLIENT Kamal  │            │  CLIENT Said   │
           │                │            │                │
           │ thread saisie  │            │ thread saisie  │  (stdin → serveur)
           │ thread recv    │            │ thread recv    │  (serveur → écran)
           └────────────────┘            └────────────────┘
```

**Côté serveur**
* Le thread principal boucle sur `accept()` et crée un `pthread` *détaché*
  par client.
* Chaque `client_thread` lit les lignes du client, met à jour l'état et
  relaie les messages aux destinataires.
* La table `clients[MAX_CLIENTS]` (fd, pseudo, horodatage, occupé) est
  partagée et protégée par un `pthread_mutex_t`.
* Pour diffuser, on **copie** les `fd` cibles sous verrou, puis on relâche le
  verrou avant les `send()` : un client lent ne bloque pas la diffusion
  (`SO_SNDTIMEO` borne chaque envoi à 5 s).

**Côté client**
* Un thread lit l'entrée clavier et envoie les commandes au serveur.
* Un thread `recv` lit en continu le socket et affiche les évènements,
  en réécrivant proprement le prompt (`\r\033[K`).

## 2. Flux des messages

### Diffusion (`MSG`)

```
  Said                     Serveur                        Kamal
   │  MSG Bonjour            │                               │
   │───────────────────────►│                               │
   │                        │ snapshot des fd (sous mutex)   │
   │                        │ relais hors verrou             │
   │                        │  MSG Said Bonjour              │
   │                        │──────────────────────────────►│
   │     (pas d'écho)       │                               │
```

L'émetteur est exclu de la diffusion (`except_fd`).

### Message privé (`PRIV`)

```
  Said                     Serveur                        Kamal
   │  PRIV Kamal Coucou      │                               │
   │───────────────────────►│ recherche du pseudo cible      │
   │                        │  PRIV Said Coucou              │
   │                        │──────────────────────────────►│
```

Si la cible n'existe pas : `ERR no such user: <nom>` renvoyé à l'émetteur.

## 3. Description des messages échangés

Lignes ASCII terminées par `\n`, premier *token* = verbe.

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

### Exemple de session brute

```
C → S : NICK Kamal
S → C : OK
S → tous : JOIN Kamal
C → S : MSG Bonjour tout le monde !
S → autres : MSG Kamal Bonjour tout le monde !
C → S : PRIV Said Salut Kamal
S → Said : PRIV Kamal Salut Kamal
C → S : LIST
S → C : LIST Kamal Said
C → S : QUIT
S → tous : LEAVE Kamal
```
