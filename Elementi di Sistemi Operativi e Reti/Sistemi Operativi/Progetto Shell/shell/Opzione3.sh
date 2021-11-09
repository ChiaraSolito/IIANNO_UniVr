#!/bin/bash
limit=$(<.config.txt)
clear
m=1
echo -e  "\e[1mOpzione 3: Ricerca utente per id.\e[m"
        ricercaid=stringa
        printf %s "Inserire l'id dell'utente da cercare: "
        read -r ricercaid

if ! grep -q "$ricercaid" etc/passwd; then
	m=0
	echo -e "\e[31mUtente NON trovato.\e[0m"        
else
	while read line
        do
		if  test $(echo $line | cut -d: -f3) -eq $ricercaid;then
                        if test $(echo $line | cut -d: -f3) -ge $limit; then
				echo -e "\e[1mL'utente cercato è:\e[0m"
                                echo -e "\e[32mNome utente:\e[0m" 
				echo $line | cut -d: -f1
				echo -e "\e[34mUserID:\e[0m"
                                echo $line | cut -d: -f3
				echo -e "\e[36mGroupID:\e[0m"
                                echo $line | cut -d: -f4
				echo -e "\e[35mHome Directory:\e[0m"
                                echo $line | cut -d: -f6
				echo -e "\e[33mShell:\e[0m"
                                echo $line | cut -d: -f7
				m=0
				break
                        else
                                echo -e "\e[31mL'uid dell'utente cercato non è tra quelli a cui puoi accedere.\e[0m"
                        fi
		fi
	done < etc/passwd
fi

if [ $m -eq 1 ]; then
	echo -e "\e[31mUtente NON trovato.\e[0m"
fi

echo "Torno al menù."

#VR443441
#Chiara SOlito
#18/11/2020
#userManager
#Progetto Bash A.A. 2020/2021

