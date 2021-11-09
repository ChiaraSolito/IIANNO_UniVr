#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <semaphore.h>

#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

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

///variabili globali
int n_ctrlC,shmID;

//giocatori
giocatore_t giocatore1,giocatore2; 

//variabili per gestire la grandezza del campo
int n,m;

//variabile per gestire il caso single player
int giocaDaSolo = 0;

//variabili per la gestione di semafori
int semServer, semClient;

//variabili per la gestione degli altri processi
int pidAvversario, pidServer;

//verifica che il comando ctrl-c sia eseguito con cognizione
void segnale(int sig){
	n_ctrlC++;

	if (n_ctrlC == 1) {

		printf("\nAlla prossima pressione di ^C il programma termina.Hai 5 secondi per terminare il programma o riprenderà da dove hai lasciato.\n");
    	sleep(5);

 	} else if(n_ctrlC == 2){
		
		//comunico agli altri processi
		kill(pidServer,SIGUSR1);
        kill(pidAvversario,SIGUSR1);

		printf("\nTermino il programma per la pressione di 2 ^C\n");

		//chiusura delle fifo
		unlink("primoPathToClientFIFO");
		if(giocaDaSolo == 0)
			unlink("secondoPathToClientFIFO");

		printf("Hai perso per abbandono!\n");

		exit(0);
	}
}

//partita abbandonata dal client
void abbandono(int sig){
	printf("\nL'avversario ha abbandonato la partita!\nVINCI A TAVOLINO\nPartita terminata. A presto!\n");
	exit(0);
}

//partita chiusa dal server
void terminazione(int sig){
	printf("\nIl server ha terminato la partita :(\n A presto!\n");
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

void modificaCampo(casella_t campo[][n],int colonna,int riga,char simbolo){
	campo[colonna][riga].valore = simbolo;
}

//calcola la Riga in cui finisce il simbolo una volta scelta la colonna
int calcolaRiga(casella_t campo[][n],int n,int mossa){
	for(int i = n - 1; i >= 0; i--){
		if(campo[mossa][i].valore == ' ')
			return i;
	}
	return -1;
}

//main
int main(int argc, char *argv[]){

	//////////    -IMPOSTAZIONI GENERALI-    //////////

	//inizializzazione variabili necessarie
	n_ctrlC = 0;
	int m_mossa = 4;

	//gestione dei segnali
    signal(SIGINT,segnale);
	signal(SIGUSR1,abbandono);
	signal(SIGXFSZ,terminazione);

	// Controlla che ci siano gli argomenti attesi e se è in modalità single player
	if (argc < 2) {
		printf("Errore.\nCommand line del client: %s <stringa nome utente> (optional ->) <NO> (modalità single player)\n", argv[0]);
		exit (1);
	} else if (argc >= 3)
		giocaDaSolo = 1;

	//creo il giocatore
	giocatore_t giocatoreClient;
	strcpy(giocatoreClient.nome,argv[1]);
	printf("Nome del giocatore inserito: %s\n",giocatoreClient.nome);

	//////////    -CREAZIONE E GESTIONE DELLE FIFO E DEI SEMAFORI-    //////////

	//Server FIFO file descriptor
	int ServerFIFO;
	ServerFIFO = open("pathToServerFIFO",O_WRONLY);
	if(ServerFIFO == -1) perror("Errore di accesso al ServerFIFO.");

	//scrive il nome del giocatore sulla fifo perché sia letto dal server
	write(ServerFIFO,giocatoreClient.nome,strlen(giocatoreClient.nome));

	if(giocaDaSolo){
		//se il giocatore sceglie la modalità single player trasferisce l'informazione al server
		write(ServerFIFO,argv[2],sizeof(argv[2]));
	} else {
		printf("In attesa di un altro giocatore...\n\n");
		sleep(5);
	}

	//scrivo anche il pid del processo
	giocatoreClient.pid = getpid();
	write(ServerFIFO,&giocatoreClient.pid,sizeof(giocatoreClient.pid));

	int turno = 0;
	//controllo se è già stata creata una FIFO Client
	int ClientFIFO;
	if( access("primoPathToClientFIFO", F_OK ) > -1 ) {

		//se c'è creo la clientFIFO_2: il giocatore è il GIOCATORE2
		if(mkfifo("secondoPathToClientFIFO",0666) == -1) perror("Errore di creazione del secondo ClientFIFO");

		//ClientFIFO file descripto
		ClientFIFO = open("secondoPathToClientFIFO",O_RDONLY);
		if( ClientFIFO == -1) perror("Errore di apertura del secondo ClientFIFO");

		//leggo il simbolo assegnato al giocatore
		read(ClientFIFO,&giocatoreClient.simbolo,sizeof(char));

		//leggo le dimensioni del campo
		read(ClientFIFO,&m,sizeof(int));
		read(ClientFIFO,&n,sizeof(int));

		//leggo il pid del server
		read(ClientFIFO,&pidServer,sizeof(int));

		//leggo il pid dell'avversario
		read(ClientFIFO,&pidAvversario,sizeof(int));
		turno = 2;

    } else {

		//se non c'è creo la clientFIFO1: il giocatore è il GIOCATORE1
		if(mkfifo("primoPathToClientFIFO",0666) == -1) perror("Errore di creazione del primo ClientFIFO");

		//ClientFIFO file descriptor
		ClientFIFO = open("primoPathToClientFIFO",O_RDONLY);
		if( ClientFIFO == -1) perror("Errore di apertura del primo CLientFIFO");

		//leggo il simbolo assegnato al giocatore
		read(ClientFIFO,&giocatoreClient.simbolo,sizeof(char));

		//leggo le dimensioni del campo
		read(ClientFIFO,&m,sizeof(int));
		read(ClientFIFO,&n,sizeof(int));

		//leggo il pid del server
		read(ClientFIFO,&pidServer,sizeof(int));

		//leggo il pid dell'avversario
		if(giocaDaSolo == 0)
			read(ClientFIFO,&pidAvversario,sizeof(int));

		turno = 1;
    }

	printf("Giocatore %s, giochi con il simbolo %c\nGiochi in un campo %i x %i\n\n",giocatoreClient.nome,giocatoreClient.simbolo,m,n);
	
	//legge la key della memoria condivisa
	int key;
	read(ClientFIFO,&key,sizeof(key));
	//accedo al segmento di memoria condivisa
 	int ShmID = shmget(key,sizeof(casella_t[m][n]),066);
	if(ShmID == -1) perror("Errore di accesso al segmento di memoria condivisa.");

	//creo un puntatore al campo
	casella_t (*campo)[n]  = shmat(ShmID,NULL,0);
	if((void *)campo == (void *)-1)perror("Errore di collegamento alla memoria condivisa.");
	
	sleep(2);

	//legge la key del semaforo
	int key_semaforo;
	read(ClientFIFO,&key_semaforo,sizeof(key_semaforo));
	//imposto il semaforo
	int semServer;
	semServer = semget(key_semaforo, 1, 0644);
	if( semServer < 0 ) {
			perror("Errore nella lettura del semaforo.");
			exit(1);
	}

	//////////    -TURNO EFFETTIVO DI GIOCO-    //////////
	do{

		if(turno == 2){

			//leggo se ho vinto o perso
			printf("Attendo che l'altro giocatore finisca il suo turno....\n");
			read(ClientFIFO,&giocatoreClient.status,sizeof(giocatoreClient.status));
			turno = 1;

		} else if (turno == 1){

			printf("Turno del giocatore %s..\n",giocatoreClient.nome);

			/////    -Il semaforo è sbloccato dall'esterno-    /////

			//stampo il campo
			stampaCampo(campo,n,m);

			//chiedo al giocatore di scegliere in quale colonna giocare
			do{
				printf("Scegli la colonna in cui inserire il tuo gettone.");
				scanf("%d",&giocatoreClient.mossa);
				printf("Metto il gettone nella colonna %d\n",giocatoreClient.mossa);
				m_mossa = calcolaRiga(campo,n,giocatoreClient.mossa - 1);

				//verifico che la colonna scelta sia nel range e che non sia piena
				if(giocatoreClient.mossa>m || giocatoreClient.mossa<1)
					printf("Errore. Colonna non presente nel campo da gioco\n");
				else if( m_mossa < 0 )
					printf("Errore.Colonna piena\n");

			}while(giocatoreClient.mossa>m || giocatoreClient.mossa<1 || m_mossa < 0);

			//modifico effettivamente il campo
			modificaCampo(campo,giocatoreClient.mossa -1,m_mossa,giocatoreClient.simbolo);

			//stampo il campo
			stampaCampo(campo,n,m);
			printf("Giocatore %s hai giocato il tuo turno.\n",giocatoreClient.nome);

			//sblocco il semaforo del server
			struct sembuf sem;
			sem.sem_op = 1;
			sem.sem_num = 0;
			sem.sem_flg = 0;
			semop(semServer,&sem,1);

			/////    -Fine delle operazioni con semafori    /////

			//leggo se ho vinto o perso
			read(ClientFIFO,&giocatoreClient.status,sizeof(giocatoreClient.status));

			if(giocaDaSolo == 0)
				turno = 2;
		}

		if(giocatoreClient.status == 1){
			printf("\n\n");
			stampaCampo(campo,n,m);
			printf("GIOCATORE %s HAI VINTO LA PARTITA!!!\n",giocatoreClient.nome);
		} else if(giocatoreClient.status == -1){
			printf("\n\n");
			stampaCampo(campo,n,m);
			printf("Oh no! Giocatore %s il tuo avversario ha vinto la partita! :(\n",giocatoreClient.nome);
		} else if(giocatoreClient.status == 2){
			printf("\n\n");
			stampaCampo(campo,n,m);
			printf("Matrice piena!La partita finisce in parità!\n");
		} 

	}while(giocatoreClient.status == 0);
	//////////    -FINE DEL TURNO DI GIOCO-    //////////

	//chiudo il collegamento alla memoria condivisa
    shmdt(campo);

	//chiudo i file descriptor
    unlink("primoPathToClientFIFO");
	unlink("secondoPathToClientFIFO");

    return(0);
}


/*******************************
* VR443441
* Chiara Solito
* Ultima modifica: 22/01/2021
* Data di creazione: 27/12/2020
* Elaborato 2 - SYSTEM CALL
*******************************/