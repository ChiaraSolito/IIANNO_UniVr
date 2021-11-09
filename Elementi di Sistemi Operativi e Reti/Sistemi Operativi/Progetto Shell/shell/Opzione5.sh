#!/bin/bash
clear
limit=$(<.config.txt)
echo -e "\e[1mOpzione 5: Cancella gruppo utenti per nome.\e[0m"
cancellanome=stringa
nomeutente=stringa
m=0
x=2

printf %s "Inserire il nome  del gruppo utenti da cancellare:"
read -r cancellanome
printf %s\\n "Hai scritto: ${cancellanome}"

if ! grep -q "$cancellanome" etc/passwd; then
	echo -e "\e[31mUtenti NON trovati.\e[0m"
else
	while read line
        do
		if  grep -q "$cancellanome" <<< "$(echo $line | cut -d: -f1)";then
                        if test $(echo $line | cut -d: -f3) -ge $limit; then
                                echo $line | cut -d: -f-1,3-4,6- | grep $cancellanome | column -t -s ":"
				m=1
                        else
                                echo -e "\e[31mL'uid degli utenti cercati non sono tra quelli a cui puoi accedere.\e[0m"
                        fi
                fi

done < etc/passwd
fi

if [ $m -eq 1 ]; then
         echo -e "\e[1mCancellare gli utenti? (inserire 1 per cancellare/inserire 0 per annullare)\e[0m"
         read x
	if [ $x -eq 1 ]; then
		while read line
        	do
                	if  grep -q "$cancellanome" <<< "$(echo $line | cut -d: -f1)";then
				nomeutente=$(echo $line | cut -d: -f1)
				bin/userdel $nomeutente
			fi
		done < etc/passwd
		echo -e "\e[1mUtenti cancellati.\e[0m"
	elif [ $x -eq 0 ]; then
		echo -e "\e[31mOperazione annullata.\e[0m"
	fi
fi

echo "Torno al menù."

#VR443441
#Chiara SOlito
#19/11/2020
#userManager
#Progetto Bash A.A. 2020/2021

