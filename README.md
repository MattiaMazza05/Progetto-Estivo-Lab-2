# 🚨 Sistema di Gestione Emergenze Concorrente

> **Università di Pisa - Laboratorio 2 – a.a. 2024/2025**  
> **Autore:** Mattia Mazza - Matricola: 671860  
> **Progetto:** Sistema concorrente per la gestione di emergenze con multithreading e message queue POSIX

## 📋 Descrizione

Sistema concorrente per la gestione di emergenze in tempo reale implementato in **C11**. Il sistema utilizza thread POSIX, mutex per la sincronizzazione e message queue POSIX per la comunicazione inter-processo.

### ✨ Funzionalità Principali

- 🔄 **Gestione concorrente**: Ogni emergenza è gestita da un thread dedicato
- 📊 **Assegnazione intelligente**: Calcolo automatico della disponibilità dei mezzi di soccorso
- 📍 **Sistema di coordinate**: Ambiente bidimensionale con calcolo distanza Manhattan
- ⏱️ **Gestione priorità**: Sistema di timeout basato sulla priorità dell'emergenza
- 🚑 **Digital Twin**: Tracciamento real-time dello stato dei mezzi di soccorso
- 📱 **Client separato**: Invio emergenze tramite terminale o file batch
- 🔐 **Thread-safe**: Protezione delle risorse condivise con mutex

## 🏗️ Architettura del Sistema

```
┌─────────────────┐    Message Queue    ┌─────────────────┐
│     CLIENT      │◄──────────────────►│     SERVER      │
│                 │     (POSIX mq)      │                 │
│ - Invio singolo │                     │ - Main thread   │
│ - Invio batch   │                     │ - Thread pool   │
│ - Terminazione  │                     │ - Mutex sync    │
└─────────────────┘                     └─────────────────┘
                                                │
                                        ┌───────┴───────┐
                                        │  COMPONENTI   │
                                        │               │
                                        │ 📊 Parser     │
                                        │ 🚑 Rescuers   │
                                        │ 🚨 Emergenze  │
                                        │ 🗺️ Environment│
                                        └───────────────┘
```

## 📁 Struttura dei File

```
src/Progetto Estivo/
├── 🔧 Core Files
│   ├── main.c                    # Server principale e gestione thread
│   ├── client.c                  # Client per invio emergenze
│   └── Makefile                  # Compilazione e gestione build
│
├── 📋 Header Files
│   ├── types.h                   # Definizione strutture dati
│   ├── emergency.h               # Header emergenze
│   ├── rescuers.h                # Header soccorritori
│   ├── env.h                     # Header ambiente
│   └── macro.h                   # Macro e costanti
│
├── 🔍 Parser Files
│   ├── parse_emergency_types.c   # Parser tipi emergenze
│   ├── parse_rescuers.c          # Parser soccorritori
│   └── parse_env.c               # Parser ambiente
│
├── ⚙️ Configuration Files
│   ├── emergency_typers.conf     # Tipi di emergenze
│   ├── rescuers.conf             # Tipi di soccorritori
│   └── env.conf                  # Configurazione ambiente
│
└── 📄 Documentation
    ├── README.md                 # Questo file
    ├── emergenze.txt             # File di esempio per batch
    └── Testo Progetto Estivo...  # Specifiche del progetto
```

## 📝 File di Configurazione

### `rescuers.conf` - Definizione Soccorritori
```properties
[Pompieri][5][20][100;200]    # [Nome][Quantità][Velocità][PosX;PosY]
[Polizia] [5] [30] [200;100]
[Ambulanza][5][40][150;250]
```

### `emergency_typers.conf` - Tipi di Emergenze
```properties
[Furto] [0] Polizia:2,3;                           # [Nome][Priorità] Tipo:Quantità,Tempo;
[Incidente] [2] Polizia:1,5;Ambulanza:2,10;Pompieri:1,3;
[Incendio] [1] Ambulanza:2,4;Pompieri:3,15;
```

### `env.conf` - Configurazione Ambiente
```properties
queue=emergenze671860    # Nome della message queue
height=500              # Altezza mappa
width=500               # Larghezza mappa
```

## 🚀 Compilazione e Esecuzione

### Prerequisiti
- **OS**: Linux (testato su Ubuntu 24.04+)
- **Compiler**: GCC con supporto C11
- **Librerie**: POSIX threads, POSIX message queues

### Compilazione
```bash
make                    # Compila server e client
# Oppure
make all               # Equivalente a make
```

### Avvio del Sistema
```bash
# Terminal 1: Avvia il server
make run               # Oppure ./server

# Terminal 2: Invia emergenze
./client Incendio 150 200 3                    # Emergenza singola
./client -f emergenze.txt                      # Emergenze da file
./client exit                                  # Termina il server
```

### Pulizia
```bash
make clean             # Rimuove eseguibili e file oggetto
```

## 💻 Utilizzo del Client

### 1. Emergenza Singola
```bash
./client <tipo_emergenza> <coord_x> <coord_y> <timestamp>
./client Incendio 150 200 1719152400
```

### 2. Emergenze da File Batch
```bash
./client -f emergenze.txt
```

Formato file `emergenze.txt`:
```
Furto 30 150 1719152400
Incendio 120 400 1719156000
Incidente 220 340 1719163200
```

### 3. Terminazione Sistema
```bash
./client exit
```

## 🔧 Algoritmo di Gestione

### 1. **Ricezione Emergenza**
- Server riceve emergenza via message queue
- Validazione coordinate e tipo emergenza
- Creazione thread dedicato per gestione

### 2. **Verifica Disponibilità**
```c
// Per ogni tipo di soccorritore richiesto
for (tipo_soccorritore) {
    // Conta mezzi disponibili (status = IDLE)
    // Calcola tempo di arrivo (distanza Manhattan)
    // Verifica se sufficienti per i requisiti
}
```

### 3. **Assegnazione Risorse**
- Se tutti i mezzi disponibili: assegna (status = EN_ROUTE_TO_SCENE)
- Calcola tempo massimo di arrivo
- Verifica rispetto al timeout di priorità
- Aggiorna status emergenza (IN_PROGRESS/TIMEOUT)

### 4. **Gestione Priorità**
```c
switch(priority) {
    case 1: timeout = 30 secondi;  // Alta priorità
    case 2: timeout = 10 secondi;  // Media priorità  
    default: timeout = 100 secondi; // Bassa priorità
}
```

## 🧵 Gestione Thread e Sincronizzazione

### Thread Pool
- **Main thread**: Gestione message queue e creazione thread emergenze
- **Emergency threads**: Un thread per ogni emergenza (gestione_emer)
- **Synchronization**: Mutex globale per protezione array mezzi

### Meccanismi di Sincronizzazione
```c
mtx_t mutex_soccorritori;           // Protezione accesso mezzi
mtx_lock(&mutex_soccorritori);      // Acquisizione risorsa
// ... operazioni critiche ...
mtx_unlock(&mutex_soccorritori);    // Rilascio risorsa
```

## 📊 Output e Debug

### Riepilogo Thread
```
========== RIEPILOGO THREAD 1 ==========
📋 EMERGENZA: Incendio
📍 POSIZIONE: (150, 200)
⚡ PRIORITÀ: 1 (limite: 30 sec)
🔍 TIPI MEZZI CONTROLLATI: 2/2
⏰ TEMPO MASSIMO CALCOLATO: 15 secondi
🚑 MEZZI TOTALI ASSEGNATI: 5
✅ TUTTI MEZZI DISPONIBILI: SÌ
⏱️ RISPETTA TEMPO LIMITE: SÌ
🟢 STATUS FINALE: IN_PROGRESS
=========================================
```

## 🏷️ Stati del Sistema

### Stati Emergenze
- `WAITING`: In attesa di assegnazione
- `ASSIGNED`: Mezzi assegnati
- `IN_PROGRESS`: Intervento in corso
- `COMPLETED`: Emergenza risolta
- `TIMEOUT`: Tempo limite superato
- `CANCELLED`: Emergenza annullata

### Stati Mezzi di Soccorso
- `IDLE`: Libero e disponibile
- `EN_ROUTE_TO_SCENE`: In viaggio verso emergenza
- `ON_SCENE`: Sul luogo dell'intervento
- `RETURNING_TO_BASE`: Ritorno alla base

## 🎯 Caratteristiche Tecniche

### Calcolo Distanza Manhattan
```c
#define DISTANZA_MANHATTAN(x1,y1,x2,y2,vel) \
    int ris1 = abs(x1-x2); \
    int ris2 = abs(y1-y2); \
    int vel = (ris1 + ris2)/vel_socc
```

### Gestione Errori
- Macro `SCALL` per chiamate di sistema
- Controllo automatico dei valori di ritorno
- Gestione graceful degli errori

### Memory Management
- Allocazione dinamica per emergenze valide
- Cleanup automatico alla terminazione
- Gestione corretta dei puntatori

## 📈 Possibili Estensioni

- [ ] **Logging avanzato**: File di log dettagliato
- [ ] **Preemption**: Interruzione per emergenze prioritarie
- [ ] **Dynamic Aging**: Promozione automatica priorità
- [ ] **Load Balancing**: Bilanciamento carico tra mezzi
- [ ] **Real-time Monitoring**: Dashboard in tempo reale
- [ ] **Geographic Information**: Integrazione mappe reali

## 🐛 Debug e Troubleshooting

### Problemi Comuni
1. **Message Queue già esistente**: `mq_unlink` prima di riavviare
2. **Segmentation Fault**: Verificare accesso agli array
3. **Deadlock**: Controllare ordine acquisizione mutex
4. **Mezzi non trovati**: Verificare configurazione rescuers.conf

### Compilazione in Debug
```bash
gcc -Wall -Wextra -g -pedantic -DDEBUG main.c parse_*.c -o server
```

## 👨‍💻 Autore

**Mattia Mazza**  
📧 Email: [email universitario]  
🎓 Corso: Laurea Triennale in Informatica  
🏛️ Università di Pisa  
📊 Matricola: 671860

## 📄 Licenza

Questo progetto è realizzato esclusivamente a scopo didattico per il corso di Laboratorio 2 dell'Università di Pisa.
[Incendio][2] Pompieri:2,8;Ambulanza:3,1;
```

### `env.conf`
```
queue=emergenze123456
height=300
width=400
```

## Compilazione e Esecuzione

Assicurati di essere su **Linux Ubuntu 24.04+**

### Compilazione
```bash
make
```

### Avvio del sistema
```bash
make run
```

### Invio emergenza da terminale
```bash
./client Incendio 150 200 3
```

### Invio emergenze da file
```bash
./client -f emergenze.txt
```

### Pulizia
```bash
make clean
```

## Log

Ogni evento viene salvato in un file di log con formato:

```
[Timestamp] [ID] [EVENTO] Messaggio
```

Esempi di eventi:

- `FILE_PARSING`
- `MESSAGE_QUEUE`
- `EMERGENCY_STATUS`
- `RESCUER_STATUS`

## Requisiti Tecnici

- C11 standard
- Thread POSIX
- Message Queue POSIX (`mq_*`)
- Protezione chiamate di sistema
- Meccanismi di sincronizzazione (mutex, semafori, condition variables)
- Gestione timeout e priorità
- Evitare deadlock e starvation (se richiesto)

## Estensioni Obbligatorie (per consegne dopo 19/05/2025)

- **Gestione Deadlock**
- **Preemption** (interruzione interventi in corso per emergenze più gravi)
- **Dynamic Priority Aging** (promozione delle emergenze a priorità superiore nel tempo)

## Autori

- Mattia Mazza
- Matricola: 671860
- Corso: Laurea Triennale in Informatica
- Università di Pisa

## Licenza

Questo progetto è realizzato esclusivamente a scopo didattico.
