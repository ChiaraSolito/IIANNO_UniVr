#!/bin/bash
clear
limit=$(<.config.txt)
echo -e "\e[1mOpzione 2: Visualizzazione utenti per nome.\e[0m"
varlogin=stringa
printf %s "Inserire il nome degli utenti da visualizzare: "
read -r varlogin
printf %s\\n "Hai scritto: ${varlogin}"

if ! grep -q "$varlogin" etc/passwd; then
	echo -e "\e[31mUtenti NON trovati\e[0m"
else
	for line in `cat etc/passwd | tr -d '[[:blank:]]' | sort -t: -k3 -n`
	do
		if grep -q "$varlogin" <<< "$(echo $line | cut -d: -f1)";then
			if test $(echo $line | cut -d: -f3) -ge $limit; then
                                echo $line | cut -d: -f-1,3-4,6- | grep $varlogin | column -t -s ":"
                        else
                                echo -e "\e[31mL'uid degli utenti cercati non sono tra quelli a cui puoi accedere.\e[0m"
                        fi
                fi
	done
fi

echo "Torno al menù."

#VR443441
#Chiara SOlito
#18/11/2020
#userManager
#Progetto Bash A.A. 2020/2021

