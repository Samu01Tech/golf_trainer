# Club trainer

![logo](assets/logo.png)

## Introduzione
Il presente documento descrive in dettaglio il funzionamento di un simulatore di golf sviluppato su piattaforma Arduino. Il sistema è progettato per analizzare il movimento dello swing utilizzando un sensore IMU BNO055, fornire feedback multisensoriale: visivo, tramite LED e interfaccia grafica, sonoro, attraverso il dispositivo collegato, e tattile, grazie a un motore a vibrazione. L'obiettivo principale è creare un'esperienza interattiva che permetta ai giocatori di migliorare la propria tecnica attraverso l'utilizzo di più stimoli sensoriali e ottenere un riscontro immediato delle loro prestazioni.

## Panoramica del Sistema
Il simulatore di golf utilizza una serie di componenti hardware che lavorano insieme per raccogliere e interpretare i dati relativi allo swing del giocatore. Il cuore del sistema è rappresentato dal sensore di movimento BNO055, il quale, è in grado di rilevare posizione, velocità e accelerazione per tutti e tre gli assi. Pertanto, è possibile monitorare l'angolazione e la velocità del colpo. Questi dati vengono elaborati in tempo reale e utilizzati per determinare la qualità del colpo eseguito.

Per garantire un'interazione costruttiva con l'utente, il simulatore integra un sistema di feedback visivo composto da una serie di LED posizionati strategicamente. Questi indicatori luminosi segnalano lo stato del colpo e l'orientamento del movimento, fornendo al giocatore informazioni utili per correggere eventuali errori. Inoltre, il sistema incorpora un motore a vibrazione che emette segnali tattili in risposta a specifiche condizioni di gioco, come un colpo ben eseguito o un errore di posizione.

## Architettura Hardware e Componenti
L'implementazione del sistema si basa su una scheda Arduino (o Teensy 4.0) che funge da unità di controllo centrale. I componenti principali utilizzati nel progetto includono:

- **Sensore IMU BNO055**: Un accelerometro e giroscopio  che misura l'orientamento del colpo in tre dimensioni.
- **EEPROM**: Utilizzata per memorizzare dati persistenti relativi alla calibrazione e ai parametri di utilizzo.
- **Sensore di effetto Hall**: Impiegato per il rilevamento del passaggio della mazza nel punto *virtuale* di contatto con la palla da golf.
- **Sensore a nastro (ribbon sensor)**: Definisce la posizione della mano durante l'esecuzione dello swing, garantendo una corretta impugnatura della mazza.
- **Sistema di feedback mani**: Questo sistema aiuta il giocatore a impugnare correttamente la mazza. Una serie di LED disposti verticalmente guida il posizionamento delle mani: se queste sono troppo alte, si illumineranno i LED rossi superiori; se sono troppo basse, si accenderanno i LED rossi inferiori. Se la posizione delle mani è corretta, i LED verdi confermeranno il posizionamento.
- **Sistema di feedback swing**: Questo sistema aiuta a interpretare la correttezza dello swing. Una serie di LED disposti orizzontalmente segnala l'orientamento del movimento: se il colpo devia troppo a destra, si accenderanno i LED rossi sulla destra; se invece è troppo inclinato a sinistra, si illumineranno i LED rossi sulla sinistra. Se lo swing è corretto e ben allineato, i LED verdi centrali si illumineranno per confermare l'esecuzione ideale del colpo.
- **Motore a vibrazione**: Implementato per fornire un feedback aptico che aiuta il giocatore a percepire il momento in cui la palla viene colpita. Questo consente di modificare lo swing in modo appropriato, ad esempio anticipando l'estensione del braccio o correggendo il movimento per ottenere un colpo più efficace.
- **Pure Data (PD)**: Software utilizzato per elaborare e generare effetti sonori in tempo reale sulla base dei dati ricevuti da Arduino.
- **Processing**: Software utilizzato per la visualizzazione grafica del colpo e dell'orientamento della mazza in tempo reale, nonchè ottenere uno storico dei colpi effettuati.

## Integrazione con Pure Data
Pure Data (PD) è un ambiente di programmazione grafica per l'elaborazione audio. Nel contesto del simulatore di golf, Pure Data riceve i dati da Arduino attraverso una connessione seriale e genera suoni sincronizzati con il movimento dello swing.

### Flusso di lavoro di Pure Data
1. **Ricezione dei dati da Arduino**: Il codice PD legge i dati inviati via seriale e li interpreta per determinare il momento dell'impatto e la qualità del colpo.
2. **Generazione degli effetti sonori**: In base ai dati ricevuti, il sistema riproduce suoni diversi, tra cui:
   - Un suono più intenso per un colpo potente
   - Un suono più ovattato per un colpo debole
   - Un effetto acustico direzionale che varia in base all’angolazione dello swing
3. **Regolazione dei parametri audio**: Il sistema consente di personalizzare gli effetti sonori regolando il volume, il tono e la riverberazione in base alle prestazioni del giocatore.

## Integrazione con Processing
Processing è un ambiente di programmazione visuale utilizzato per la rappresentazione grafica dei dati ricevuti da Arduino. Nel contesto del simulatore di golf, il codice Processing legge i dati dallo swing e fornisce una visualizzazione in tempo reale dell'orientamento della mazza e della qualità del colpo.

### Flusso di lavoro di Processing
1. **Connessione con Arduino**: Processing stabilisce una connessione seriale per ricevere i dati dei sensori.
2. **Visualizzazione del movimento**: Il sistema genera un'animazione che mostra l'angolazione e la traiettoria dello swing.
3. **Feedback visivo avanzato**: Il programma può includere indicatori di posizione, angoli e vettori per segnalare eventuali errori nello swing e suggerire correzioni.

## Logica di Funzionamento
Il codice implementa una serie di routine per acquisire e interpretare i dati provenienti dai sensori e fornire un riscontro immediato al giocatore. La sequenza operativa è la seguente:

1. **Inizializzazione del sistema**: Durante la fase di avvio, il microcontrollore configura i sensori e verifica la loro corretta calibrazione. Se necessario, i dati di calibrazione vengono caricati dalla memoria EEPROM.
2. **Monitoraggio del movimento**: Il sensore IMU acquisisce costantemente dati riguardanti l'orientamento e la velocità della mazza. Questi valori vengono confrontati con soglie predefinite per determinare la qualità del colpo.
3. **Attivazione dei feedback visivi e tattili**: In base ai dati acquisiti, il sistema rappresenta delle informazioni nell'interfaccia connessasia di tipo visivo che audio. Inoltre,si attivano dei feedback visivi sulla mazza.


## Conclusioni
Il simulatore di golf basato su Arduino rappresenta una soluzione innovativa per l’allenamento e il miglioramento della tecnica di gioco. Grazie alla combinazione di sensori avanzati e sistemi di feedback multisensoriali, l’utente può affinare il proprio swing con un supporto costante e immediato. 
