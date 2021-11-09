#!/bin/bash
if [ "$1" == "--help" ]; then
  echo "Usage: userManager.sh [noOptions]"
  echo "Il programma userManager fronisce una serie di opzioni per la gestione degli utenti."
  echo "Permette la visualizzazione dell'elenco utenti completo, o tramite nome; la creazione e cancellazione di utenti o gruppi utenti, ecc."
exit 0
fi


m=1
while [ $m -lt 8 ]
do
clear
	echo "1) Visualizza elenco utenti completo"
	echo "2) Visualizza elenco utenti per nome"
	echo "3) Ricerca utente per id"
	echo "4) Cancella utente per id"
	echo "5) Cancella utenti per nome"
	echo "6) Crea singolo utente"
	echo "7) Crea gruppo di utenti"
	echo "8) Esci"
	echo "Digita un valore tra 1 e 8"
	read m
	if [ $m -gt 8 ]; then 
		echo "Errore. Inserito un valore non valido. Terminazione del programma"
	fi
	case $m in
		1) echo "Hai selezionato 1"; clear;  ./Opzione1.sh; read;;
		2) echo "Hai selezionato 2"; clear; ./Opzione2.sh; read;;
		3) echo "Hai selezionato 3"; clear; ./Opzione3.sh; read;;
		4) echo "Hai selezionato 4"; clear; ./Opzione4.sh; read;;
		5) echo "Hai selezionato 5"; clear; ./Opzione5.sh; read;;
		6) echo "Hai selezionato 6"; clear; ./Opzione6.sh; read;;
		7) echo "Hai selezionato 7"; clear; ./Opzione7.sh; read;;
		8) echo "Hai selezionato 8 - exit";;
		esac
	done
echo "Terminato"

#VR443441
#Chiara SOlito
#18/11/2020
#userManager
#Progetto Bash A.A. 2020/2021

