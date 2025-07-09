# Sistema di Gestione delle Emergenze
**Progetto di Laboratorio 2 — a.a. 2024-2025**  
Multithreading e Code di Messaggi in C11

![Build](https://img.shields.io/badge/build-passing-brightgreen)
![License](https://img.shields.io/badge/license-MIT-blue)
![Ubuntu](https://img.shields.io/badge/Ubuntu-24.04-orange)

## 📌 Obiettivo

Progettare e implementare un sistema multithread in C11 per la gestione di emergenze all’interno di una griglia 2D. Le emergenze vengono gestite tramite una coda di messaggi POSIX e richiedono l’intervento di risorse come pompieri, ambulanza, ecc., secondo la priorità e con rispetto dei vincoli temporali. Il sistema deve prevenire deadlock e starvation (se richiesto).

---

## 📁 File di Configurazione

### `rescuers.conf`

Formato:
```
[Tipo][Numero][Velocità][x;y]
```
Esempio:
```
[Pompieri][5][2][100;200]
[Ambulanza][25][4][150;250]
```

### `emergency_types.conf`

Formato:
```
[TipoEmergenza] [Priorità] Tipo1:numero1,durata1;Tipo2:numero2,durata2;
```
Esempio:
```
[Incendio] [2] Pompieri:2,8;Ambulanza:3,1;
```

### `env.conf`

Contiene il nome della coda e le dimensioni della griglia:
```
queue=emergenze123456
height=300
width=400
```

---

## 📦 Strutture Dati Principali

```c
typedef enum { IDLE, EN_ROUTE_TO_SCENE, ON_SCENE, RETURNING_TO_BASE } rescuer_status_t;

typedef struct {
  char *rescuer_type_name;
  int speed;
  int x, y;
} rescuer_type_t;

typedef struct {
  int id;
  int x, y;
  rescuer_type_t *rescuer;
  rescuer_status_t status;
} rescuer_digital_twin_t;

typedef struct {
  rescuer_type_t *type;
  int required_count;
  int time_to_manage;
} rescuer_request_t;

typedef struct {
  short priority;
  char *emergency_desc;
  rescuer_request_t *rescuers;
  int rescuers_req_number;
} emergency_type_t;

typedef enum {
  WAITING, ASSIGNED, IN_PROGRESS, PAUSED, COMPLETED, CANCELED, TIMEOUT
} emergency_status_t;

typedef struct {
  char emergency_name[64];
  int x, y;
  time_t timestamp;
} emergency_request_t;

typedef struct {
  emergency_type_t type;
  emergency_status_t status;
  int x, y;
  time_t time;
  int rescuer_count;
  rescuer_digital_twin_t* rescuers_dt;
} emergency_t;
```

---

## ⚙️ Componente Client

Il client invia emergenze alla coda:
```bash
./client <nome_emergenza> <x> <y> <delay>
```

Oppure:
```bash
./client -f <file>
```
Dove ogni riga del file ha il formato:
```
<nome_emergenza> <x> <y> <delay>
```

---

## 🧠 Log di Esecuzione

Formato:
```
[Timestamp] [ID] [Evento] Messaggio
```

Categorie evento consigliate:
- `FILE_PARSING`
- `MESSAGE_QUEUE`
- `EMERGENCY_STATUS`
- `RESCUER_STATUS`

Eventi da registrare:
- Parsing file
- Ricezione e validazione richieste
- Cambi stato emergenze/soccorritori
- Timeout
- Assegnazioni

---

## 📐 Requisiti Tecnici

- Compatibilità con **Ubuntu 24.04.1 LTS**
- Codice indentato e commentato
- Error checking su tutte le system call
- Uso di **mutex**, **semafori**, **condition variable**
- Utilizzo di **message queue POSIX**

---

## 🚨 Priorità delle Emergenze

| Priorità | Significato       | Timeout (sec) |
|----------|-------------------|---------------|
| 0        | Bassa             | Nessun limite |
| 1        | Media             | 30            |
| 2        | Alta              | 10            |

Se non gestita in tempo: lo stato diventa `TIMEOUT`.

---

## 📏 Calcolo della Distanza

Utilizzare la distanza di Manhattan:
```
d = |x1 - x2| + |y1 - y2|
tempo_arrivo = d / velocità
```

---

## 🛑 Deadlock (solo dopo il 19/05/2025)

Se consegnato dopo il primo appello, è **obbligatoria la gestione del deadlock**, tramite:
- Prevenzione
- Rilevamento e risoluzione

---

## 🔁 Estensioni (per chi non ha superato i compitini)

- **Preemption**: interrompere emergenze in corso per gestirne di più urgenti
- **Aging**: aumentare dinamicamente la priorità per evitare starvation

---

## 🚀 Come Compilare ed Eseguire

```bash
make
make run
```

---

## 📦 Consegna

La consegna consiste in un archivio `.zip` contenente:
- Codice sorgente (`.c`, `.h`, `.conf`)
- Makefile (con `make` e `make run`)
- Report PDF (max 10 pagine) con:
  - Architettura del sistema
  - Scelte progettuali
  - Istruzioni per compilazione ed esecuzione

---

## 👤 Autore

> Mattia Mazza 671860 m.mazza10@studenti.unipi.it
