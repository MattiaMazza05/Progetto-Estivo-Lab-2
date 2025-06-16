# Laboratorio2-Primo

## Descrizione del progetto

Questo progetto gestisce la configurazione e la simulazione di mezzi di soccorso e tipologie di emergenze, leggendo i dati da file di configurazione e popolando strutture dati in C. Il progetto è suddiviso in più file sorgente e header per una migliore organizzazione e modularità.

---

## Struttura dei file

### File di configurazione

- **rescuers.conf**  
  Contiene la configurazione dei mezzi di soccorso disponibili.  
  Esempio:
  ```
  [Pompieri][5][2][100;200]
  [Ambulanza][25][4][150;250]
  ```

- **emergency_typers.conf**  
  Contiene la configurazione delle tipologie di emergenza e i mezzi richiesti per ciascuna.  
  Esempio:
  ```
  [Allagamento] [1] Pompieri:1,5;Ambulanza:1,2;
  [Incendio] [2] Pompieri:2,8;Ambulanza:3,1;
  [Sommossa] [2] Polizia:3,8;Ambulanza:2,4;
  ```

---

### File header

- **types.h**  
  Definisce tutte le strutture dati (`struct`) ed enum utilizzate nel progetto, come `rescuer_type_t`, `rescuer_digital_twin_t`, `rescuer_request_t`, `emergency_type_t`, e `rescuers_status_t`.

- **rescuers.h**  
  Dichiara le variabili globali `mezzi_totali` e `tot_mezzi` (array dinamico dei mezzi e relativo contatore), rendendole accessibili agli altri file tramite la parola chiave `extern`.

- **macro.h**  
  Contiene macro utili per la gestione delle chiamate di sistema (`SCALL`, `SCALLREAD`) e per facilitare l’estrazione dei dati dai buffer di lettura.

---

### File sorgente

- **parse_rescuers.c**  
  Legge il file `rescuers.conf`, estrae i dati dei mezzi di soccorso e popola l’array globale dinamico `mezzi_totali`.  
  Stampa una verifica dei dati caricati.

- **parse_emergency_types.c**  
  Legge il file `emergency_typers.conf`, estrae le tipologie di emergenza e le richieste di mezzi per ciascuna emergenza, popolando un array dinamico di `emergency_type_t`.  
  Stampa una verifica dei dati caricati e accede anche all’array globale dei mezzi (`mezzi_totali`).

- **parse_env.c**  
  (Attualmente vuoto)  
  File predisposto per eventuali estensioni o parsing di altri tipi di configurazione ambientale.

- **main.c**  
  (Attualmente vuoto)  
  File predisposto per un eventuale punto di ingresso unico del programma.

---

### Makefile

Permette di compilare separatamente i due parser (`parse_r` e `parse_e`) e di eseguire entrambi tramite il comando `make run`.  
Comandi principali:
- `make` — Compila entrambi gli eseguibili.
- `make run` — Esegue entrambi gli eseguibili.
- `make clean` — Rimuove gli eseguibili e i file oggetto.

---

## Note d’uso

- Per compilare:  
  ```sh
  make
  ```
- Per eseguire entrambi i parser:  
  ```sh
  make run
  ```
- Per pulire la cartella:  
  ```sh
  make clean
  ```

---

## Dipendenze

- Compilatore C (es: `gcc`)
- Sistema operativo compatibile con le chiamate POSIX (`open`, `read`, ecc.)

---

## Autore

Laboratorio2-Primo — Progetto didattico per la gestione di emergenze e mezzi di soccorso in C.
