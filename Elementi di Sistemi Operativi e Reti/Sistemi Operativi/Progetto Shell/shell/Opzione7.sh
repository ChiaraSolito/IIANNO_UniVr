#!/bin/bash
clear
limit=$(<.config.txt)
echo -e "\e[1mOpzione 7: Crea gruppo di utenti.\e[0m"
	numUtenti=0
        UID_base=stringa
        GID=stringa
        PASS=stringa
        SHELL=stringa
        nome_base=stringa
        
	echo -e "\e[36mQuanti utenti deve contentere il gruppo?\e[0m"
	read -r numUtenti
	echo  -e "\e[36mInserire lo username di base:\e[0m"
        read -r nome_base
        echo  -e "\e[36mInserire lo userid di base:\e[0m"
        read -r UID_base
        echo  -e "\e[36mInserire il groupid (uguale per tutti):\e[0m"
        read -r GID
        echo  -e "\e[36mInserire la shell (uguale per tutti):\e[0m"
        read -r SHELL
	echo  -e "\e[36mInserire la home di base:\e[0m"
	read -r HOME_base
        echo  -e "\e[1mLa password verrà generata casualmente.\e[0m"

if [ $UID_base -ge $limit ]; then
	for (( c=1; c<=numUtenti; c++ ))
	do
		USERNAME=$nome_base$c
		HOME=$HOME_base$c
		nuovoUID=$(($UID_base+$c-1))
		PASS=`< /dev/urandom tr -cd "[A-Za-z0-9_]" | head -c 10`
		bin/useradd -l -u $nuovoUID -g $GID -p $PASS -s $SHELL -d $HOME $USERNAME
		(echo $USERNAME,$PASS)>> .pass.txt
	done
	echo "Utenti creati."
else
	echo -e "\e[31mGli uid che si sta cercando di creare, non sono tra quelli a cui puoi accedere. Termino il programma.\e[0m"
fi

echo "Torno al menù."

#VR443441
#Chiara SOlito
#19/11/2020
#userManager
#Progetto Bash A.A. 2020/2021
