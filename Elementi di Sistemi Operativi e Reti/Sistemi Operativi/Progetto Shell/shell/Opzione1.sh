#!/bin/bash
clear
echo -e  "\e[1mOpzione 1: Visualizza elenco utenti completo.\e[0m"
limit=$(<.config.txt)
for line in `cat etc/passwd | tr -d '[[:blank:]]' | sort -t: -k3 -n`
do
        if test $(echo $line | cut -d: -f3) -ge $limit; then
                echo $line | cut -d: -f-1,3-4,6- | column -s ":" -t
        fi
done

echo "Torno al menù."


#VR443441
#Chiara SOlito
#18/11/2020
#userManager
#Progetto Bash A.A. 2020/2021
