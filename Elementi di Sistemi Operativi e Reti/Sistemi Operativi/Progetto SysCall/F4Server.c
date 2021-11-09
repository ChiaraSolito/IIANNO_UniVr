#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>

//struttura dei giocatori
typedef struct giocatore_s{
    char nome[50];
    int status;
    char simbolo;
    int mossa;
	int pid;
}giocatore_t;

//il campo da gioco è costituito da un array bidimensionale di caselle
//le due dimensioni sono righe e colonne
//la struct casella contiene invece un valore, un id e l'id della casella successiva in diagonale
typedef struct casella_s{
    char valore;
    int id;
    int idDiagonale;
}casella_t;

//variabili globali
int n_ctrlC,shmID;

//giocatori
giocatore_t giocatore1,giocatore2; 

//variabili per gestire la grandezza del campo
int n,m;

//variabile per gestire il caso single player
int giocaDaSolo = 0;

//variabili per la gestione di semafori
int semServer, semClient;

//verifica che il comando ctrl-c sia eseguito con cognizione
void segnale(int sig){
	n_ctrlC++;
	if (n_ctrlC == 1) {
		printf("\nAlla prossima pressione di ^C il programma termina.Hai 5 secondi per terminare il programma o riprenderà da dove hai lasciato.\n");
    	sleep(5);
	} else if (n_ctrlC == 2){
		printf("\nTermino il programma per la pressione di 2 ^C\n");

		//elimino la memoria condivisa
	    shmctl(shmID,IPC_RMID,NULL);

		//rimuovo i semafori
        if((semctl(semServer,0,IPC_RMID,0))==-1) perror("Problemi nella chiusura del semaforo.");

		unlink("pathToServerFIFO");
		unlink("primoPathToClientFIFO");
		kill(giocatore1.pid,SIGXFSZ);

		if(giocaDaSolo == 0){
			unlink("secondoPathToClientFIFO");
			kill(giocatore2.pid,SIGXFSZ);
		}

		printf("Partita terminata! A presto!\n");
		exit(0);

	}
}

//partita abbandonata dal client
void abbandono(int sig){

	printf("\nIl client ha abbandonato la partita.\n");
	if(giocaDaSolo == 1)
		printf("Il computer vince a tavolino!\n");
	unlink("pathToServerFIFO");

	//elimino la memoria condivisa
    shmctl(shmID,IPC_RMID,NULL);

    //rimuovo i semafori
    if((semctl(semServer,0,IPC_RMID,0))==-1) perror("Problemi nella chiusura del semaforo.");

	exit(0);
}

//stampa il campo da gioco
void stampaCampo(casella_t campo[][n],int n,int m){
    for( int i = 0; i < n; i++ ){
        for( int j = 0; j < m; j++ )
            printf("| %c  ",campo[j][i].valore);
        printf("|\n");
        for( int j = 0; j <= m; j++ )
            printf("----");
        printf("-\n");
    }
}

//controlla se c'è un vincitore nelle righe
char controllaRiga(casella_t campo[][n],int n,int m){
	int conta = 1;
	for(int j = n - 1; j >= 0; j--){
		for(int i = 0; i < m; i++){
			if((campo[i][j].valore == campo[i + 1][j].valore) && (campo[i][j].valore != ' '))
				conta++;
			else
				conta = 1;
			if(conta == 4)
                return campo[i][j].valore;
		}
	}
	return 0;
}

//controlla se c'è un vincitore nelle colonne
char controllaColonna(casella_t campo[][n],int n,int m){
    int conta = 1;
	for(int i = 0; i < m; i++){
        for(int j = n - 1; j >= 0; j--){
            if((campo[i][j].valore == campo[i][j + 1].valore) && (campo[i][j].valore != ' '))
                conta++;
            else
                conta = 1;
			if(conta == 4)
                return campo[i][j].valore;
		}
    }
    return 0;
}

//controlla se la matrice è piena
int matricePiena(casella_t campo[][n],int n,int m){
        for(int j = 0; j < m; j++){
            for(int i = 0; i < n; i++){
                if(campo[j][i].valore == ' ')
                    return 0;
            }
        }
	return 1;
}


int creaChiave(){
	// Crea la chiave
	key_t shm_key = ftok("./F4Server.c", 300);

	// Controlla che ftok sia andata a buon fine
	if(shm_key == -1)perror("Errore nell'esecuzione di ftok\n");
	
	return shm_key;
}

//inizializza il campo in un campo vuoto
void campoVuoto(casella_t campo[][n],int n,int m){
	int idMaker = 1;
	for( int i = 0; i < n; i++ ){
		for( int j = 0; j < m; j++, idMaker++){
			campo[j][i].id = idMaker;
			campo[j][i].idDiagonale = campo[j][i].id + m + 1;
			campo[j][i].valore = ' ';
		}
	}
}

//calcola la Riga in cui finisce il simbolo una volta scelta la colonna
int calcolaRiga(casella_t campo[][n],int n,int mossa){
	for(int i = n - 1; i >= 0; i--){
		if(campo[mossa][i].valore == ' ')
			return i;
	}
	return -1;
}

int main(int argc, char *argv[]){

	//////////    -IMPOSTAZIONI DI BASE-    //////////

	// Controlla che ci sia l'argomento nella funzione
	if (argc < 4) {
		printf("Errore.\nCommand line del server: %s <int num colonne> <int num righe> <char simbolo giocatore 1> < char simbolo giocatore 2>\n", argv[0]);
		exit(1);
	}

	//gestione dei segnali
	signal(SIGINT,segnale);
	signal(SIGUSR1,abbandono);

	//inizializzazione variabili necessarie
	int n_crtC = 0;
	m = atoi(argv[1]);
	n = atoi(argv[2]);

	//verifica che le dimensioni del campo siano maggiori o uguali a 5x5
	if(n<5 || m<5){
		do{
			printf("Il campo da gioco non è abbastanza grande.\nInserire un numero di righe e colonne maggiore o uguale di 5.\nColonne:\n");
			scanf("%d",&m);
			printf("Righe:\n");
			scanf("%d",&n);
		}while(n<5 || m<5);
	}

	//creazione ed inizializzazione dei due giocatori
	giocatore1.simbolo = argv[3][0];
	giocatore2.simbolo = argv[4][0];
	giocatore1.status = 0;
	giocatore2.status = 0;

	//apro un segmento di memoria condivisa
	int key = creaChiave();
	shmID = shmget(key,sizeof(casella_t[m][n]),IPC_CREAT | 0666);
	if(shmID == -1) perror("Errore nell'apertura della memoria condivisa");

	//attacco al segmento di memoria condivisa il campo da gioco con m righe ed n colonne, il campo è un array di caselle
	casella_t (*campo)[n]  = shmat(shmID,NULL,0);
	if((void *)campo == (void *)-1)perror("Errore nell'attaccarsi alla memoria condivisa");

	//inizializzo il campo a campo vuoto
	campoVuoto(campo,n,m);

	//////////    -CREAZIONE E GESTIONE DELLE FIFO E DEI SEMAFORI-    //////////

	//creo FifoServer
	int ServerFIFO;
	if(mkfifo("pathToServerFIFO",0666) == -1) perror("Errore nella creazione di ServerFIFO");

	//FIFOSERVER file descriptor
	ServerFIFO = open("pathToServerFIFO",O_RDWR);
	if( ServerFIFO == -1) perror("Errore nell'apertura del ServerFIFO");

	printf("Sto per leggere il nome dei  giocatori...\n");
	//leggi dalla FIFO il nome dei giocatori
	read( ServerFIFO, giocatore1.nome, sizeof(giocatore1.nome));
	read( ServerFIFO, giocatore2.nome, sizeof(giocatore2.nome));

	//controlla che la seconda stringa non sia NO altrimenti il giocatore gioca contro il computer
	if(strcmp(giocatore2.nome,"NO") == 0){

		printf("GIOCATORE %s CONTRO IL COMPUTER!\n",giocatore1.nome);
		giocaDaSolo = 1;

		//leggo i pid del processo dell'unico giocatore
		read( ServerFIFO, &giocatore1.pid, sizeof(giocatore1.pid));

	} else {

		printf("SI SFIDANO IN QUESTA PARTITA.\nGiocatore 1: %s\nGiocatore 2: %s\n",giocatore1.nome,giocatore2.nome);

		//leggo i pid dei processi dei giocatori
		read( ServerFIFO, &giocatore1.pid, sizeof(giocatore1.pid));
		read( ServerFIFO, &giocatore2.pid, sizeof(giocatore2.pid));

	}

	sleep(3);
	int pid = getpid();

	//ClientFIFO file descriptor
	int ClientFIFO_1;
	ClientFIFO_1 = open("primoPathToClientFIFO",O_WRONLY);
	if(ClientFIFO_1 == -1) perror("Errore nell'apertura del primo file descriptor Client");

	//comunica il simbolo assegnato al primo giocatore e le dimensioni del campo
	write(ClientFIFO_1,&giocatore1.simbolo,1);
	write(ClientFIFO_1,&m,sizeof(m));
	write(ClientFIFO_1,&n,sizeof(n));
	write(ClientFIFO_1,&pid,sizeof(pid));

	//se c'è un secondo giocatore scrive il secondo simbolo
	int ClientFIFO_2;
	if(giocaDaSolo == 0){
		ClientFIFO_2 = open("secondoPathToClientFIFO",O_WRONLY);
		if(ClientFIFO_2 == -1) perror("Errore nell'apertura del secondo file descriptor Client\n");

		//comunica il simbolo assegnato al secondo giocatore, le dimensioni del campo e il pid del server
		write(ClientFIFO_2,&giocatore2.simbolo,sizeof(char));
		write(ClientFIFO_2,&m,sizeof(m));
		write(ClientFIFO_2,&n,sizeof(n));
		write(ClientFIFO_2,&pid,sizeof(pid));

		//comunica ad entrambi i giocatori il pid dell'avversario
		write(ClientFIFO_1,&giocatore2.pid,sizeof(giocatore2.pid));
		write(ClientFIFO_2,&giocatore1.pid,sizeof(giocatore1.pid));

		//comunica ad entrambi i giocatori la chiave 
		write(ClientFIFO_1,&key,sizeof(key));
		write(ClientFIFO_2,&key,sizeof(key));

	} else {
		//altrimenti crea un giocatore computer contro cui giocare
		write(ClientFIFO_1,&key,sizeof(key));
		strcpy(giocatore2.nome,"Computer");
	}

	//altrimenti apro il semaforo e lo imposto a zero per permettere al client di giocare il proprio turno
	key = creaChiave();

	//comunica ad entrambi i giocatori la chiave 
	write(ClientFIFO_1,&key,sizeof(key));
	if(giocaDaSolo == 0)
		write(ClientFIFO_2,&key,sizeof(key));
	semServer = semget(key, 1, IPC_CREAT | IPC_EXCL | 0644);
	struct sembuf sem;
	sem.sem_op = 0;
	sem.sem_num = 0;
	sem.sem_flg = 0;
	semop(semServer,&sem,1);


	//variabile per la gestione della vittoria
	char vincitore;

	//////////    -CICLO DI GIOCO-    //////////
	printf("Inizia la partita!\n");
	do{
		sem.sem_op = -1;
        semop(semServer,&sem,1);

		//controllo se la matrice e piena
		if(matricePiena(campo,n,m)){

			//esco dal gioco, non vince nessuno
			printf("La partita finisce in parità perché la matrice è piena!\n");

			//comunico ai client
			giocatore1.status = 2;
			write(ClientFIFO_1,&giocatore1.status,sizeof(giocatore1.status));
			giocatore2.status = 2;
			write(ClientFIFO_2,&giocatore2.status,sizeof(giocatore2.status));

		} else {

			//controllo se qualcuno ha vinto
			vincitore = controllaColonna(campo,n,m);
			if(vincitore == 0)
				vincitore = controllaRiga(campo,n,m);

			//comunica ai client se qualcuno ha vinto o perso	
			if(vincitore){
				
				if(vincitore == giocatore1.simbolo){
					giocatore1.status = 1;
					write(ClientFIFO_1,&giocatore1.status,sizeof(giocatore1.status));

					if(giocaDaSolo == 0){
						giocatore2.status = -1;
						write(ClientFIFO_2,&giocatore2.status,sizeof(giocatore2.status));
						printf("Gioco terminato!\nVince: %s\nPerde: %s\nA presto!\n",giocatore1.nome,giocatore2.nome);
					} else 
						printf("Gioco terminato!\nIl giocatore %s ha vinto contro il computer!\nA presto!\n",giocatore1.nome);
			
				} else {
					giocatore1.status = -1;
					write(ClientFIFO_1,&giocatore1.status,sizeof(giocatore1.status));

					if(giocaDaSolo == 0){
						giocatore2.status = 1;
						write(ClientFIFO_2,&giocatore2.status,sizeof(giocatore2.status));
						printf("Gioco terminato!\nVince: %s\nPerde: %s\nA presto!\n",giocatore2.nome,giocatore1.nome);
					} else
						printf("Gioco terminato! Ha vinto il computer contro %s!\n",giocatore1.nome);
				}

			} else {

				//se in modalità single player, muove il computer casualmente, altrimenti comunica anche all'altro client
				if(giocaDaSolo){
					int riga;
					do{
						srand(time(NULL));
						int mossa = rand() % m;
						riga = calcolaRiga(campo,n,mossa);

						//modifico effettivamente il campo
						if(riga != -1){
							campo[mossa][riga].valore = giocatore2.simbolo;
							stampaCampo(campo,n,m);
							printf("Turno del giocatore...\n");
						}

					}while(riga == -1);
					
				} else {
					giocatore2.status = 0;
					write(ClientFIFO_2,&giocatore2.status,sizeof(giocatore2.status));
				}

				//altrimenti comunica loro di continuare a giocare
				giocatore1.status = 0;
				write(ClientFIFO_1,&giocatore1.status,sizeof(giocatore1.status));

			}

		}

	}while(giocatore1.status == giocatore2.status && giocatore1.status == 0);

	//////////    -CHIUSURA DELLA PARTITA-    //////////

	//chiudo le FIFO
	unlink("pathToServerFIFO");
	if(close(ClientFIFO_1) == -1) perror("Errore nella chiusura del primo file descriptor Client.");
	if(giocaDaSolo == 0)
		if(close(ClientFIFO_2) == -1) perror("Errore nella chiusura del secondo file descriptor Client.");

	//elimino la memoria condivisa
	shmctl(shmID,IPC_RMID,NULL);

	//rimuovo i semafori
	if((semctl(semServer,0,IPC_RMID,0))==-1) perror("Problemi nella chiusura del semaforo.");

    return(0);

}

/*******************************
* VR443441
* Chiara Solito
* Ultima modifica: 23/01/2021
* Data di creazione: 27/12/2020
* Elaborato 2 - SYSTEM CALL
*******************************/