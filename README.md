# Sistema di Gestione delle Emergenze in C11

> **Università di Pisa - Laboratorio 2 – a.a. 2024/2025**  
> Progetto: Sistema concorrente per la gestione di emergenze con multithreading e code di messaggi POSIX

## Descrizione

Questo progetto implementa un sistema concorrente per la gestione di emergenze all'interno di un ambiente bidimensionale simulato. Il sistema è realizzato in C11 e utilizza thread POSIX, meccanismi di sincronizzazione (mutex, semafori, condition variable) e una coda di messaggi POSIX per la comunicazione tra processi.

Il sistema assegna in modo dinamico e prioritario le risorse di soccorso (pompieri, ambulanze, ecc.) alle emergenze, rispettando i vincoli temporali, evitando deadlock e, se richiesto, prevenendo la starvation.

## Funzionalità

- Parsing da file di configurazione (soccorritori, emergenze, ambiente)
- Gestione concorrente delle emergenze
- Assegnazione automatica dei soccorritori tramite digital twin
- Simulazione della mappa con sistema di coordinate
- Calcolo della distanza di Manhattan e gestione del tempo di intervento
- Logging dettagliato di tutti gli eventi significativi
- Client separato per l'invio delle emergenze tramite coda di messaggi
- Preemption e aging per evitare starvation (opzionali in base al requisito)

## Struttura dei File

```
├── client.c                  # Generatore di emergenze (invio su message queue)
├── main.c                   # Core del sistema di gestione
├── emergency_manager.c      # Gestione delle emergenze e assegnazione soccorritori
├── logger.c                 # Gestione centralizzata del file di log
├── parse_rescuers.c         # Parser per rescuers.conf
├── parse_emergency_types.c  # Parser per emergency_types.conf
├── parse_env.c              # Parser per env.conf
├── rescuers.conf            # File di configurazione dei tipi di soccorritori
├── emergency_types.conf     # File di configurazione dei tipi di emergenze
├── env.conf                 # File di configurazione ambiente e coda
├── makefile                 # Per build, esecuzione e pulizia
└── README.md                # Questo file
```

## File di Configurazione

### `rescuer.conf`
```
[Pompieri][5][2][100;200]
[Ambulanza][25][4][150;250]
```

### `emergency_types.conf`
```
[Allagamento][1] Pompieri:1,5;Ambulanza:1,2;
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

- [TUO NOME]
- Matricola: [123456]
- Corso: Laurea Triennale in Informatica
- Università di Pisa

## Licenza

Questo progetto è realizzato esclusivamente a scopo didattico.
