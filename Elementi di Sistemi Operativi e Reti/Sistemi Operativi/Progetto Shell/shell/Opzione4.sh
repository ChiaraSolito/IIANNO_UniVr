#!/bin/bash
clear
x=0
m=1
limit=$(<.config.txt)

echo -e "\e[1mOpzione 4: Cancella utente per id.\e[0m"
        ricercaid=stringa
        printf %s "Inserire l'id dell'utente da cancellare: "
        read -r ricercaid

if ! grep -q "$ricercaid" etc/passwd; then
	echo -e "\e[31mUtente NON trovato.\e[0m"
	m=0
else 
	while read -u 3 line
        do	
		if  test $(echo $line | cut -d: -f3) -eq $ricercaid;then
                        if test $(echo $line | cut -d: -f3) -ge $limit; then
				m=0
                                nomeutente=$(echo $line | cut -d: -f1)
                                echo "Visualizzo i dati dell'utente."
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

                                echo "Cancellare l'utente? (inserire 1 per cancellare/inserire 0 per annullare)"
        			read x
        			if [ $x -eq 1 ]; then
               				bin/userdel $nomeutente
               				echo -e "\e[31mUtente cancellato.\e[0m"
        			elif [ $x -eq 0 ]; then
               				echo -e "\e[31mOperazione annullata. Programma terminato.\e[0m"
       				else
					echo "Inserito un valore non valido. Termino il programma."
	                	fi
				break
			else
                                echo -e "\e[31mL'uid dell'utente cercato non è tra quelli a cui puoi accedere.\e[0m"
                        fi
               fi
	done 3<etc/passwd
fi

if [ $m -eq 1 ]; then
        echo -e "\e[31mUtente NON trovato.\e[0m"
fi

echo "Torno al menù."

#VR443441
#Chiara SOlito
#19/11/2020
#userManager
#Progetto Bash A.A. 2020/2021

