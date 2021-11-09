#!/bin/bash
clear
limit=$(<.config.txt)
m=0
echo -e "\e[1mOpzione 6: Crea un singolo utente.\e[0m"
        nuovoUID=stringa
	GID=stringa
	PASS=stringa
	SHELL=stringa
	HOME=stringa
	USERNAME=stringa
        echo -e "\e[36mCreo un utente. Inserire lo username da assegnare:\e[0m"
        read -r USERNAME
	echo  -e "\e[36mInserire lo userid:\e[0m"
        read -r nuovoUID
	echo  -e "\e[36mInserire il groupid:\e[0m"
        read -r GID
	echo  -e "\e[36mInserire la shell:\e[0m"
        read -r SHELL
	echo  -e "\e[36mInserire la home:\e[0m"
        read -r HOME
	echo  -e "\e[36mInserire la password per l'utente:\e[0m"
        read -s PASS
	
	if [ $nuovoUID -ge $limit ]; then
		bin/useradd -u $nuovoUID -g $GID -p $PASS -s $SHELL -d $HOME -l $USERNAME
		echo "L'utente appena creato è:"
                cat etc/passwd | cut -d: -f-1,3-4,6- | grep $nuovoUID | column -t -s ":"
	else
		echo -e "\e[31mL'uid che si sta cercando di creare, non è tra quelli a cui puoi accedere. Termino il programma.\e[0m"
	fi

echo "Torno al menù."

#VR443441
#Chiara SOlito
#19/11/2020
#userManager
#Progetto Bash A.A. 2020/2021

