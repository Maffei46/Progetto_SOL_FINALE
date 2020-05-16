#!/bin/sh
#Formattazione testo
echo "\e[43m\e[30m           -> Iniziata  Analisi <-         \e[49m\e[39m"

#Dichiarazione Variabili txt
LOGFILECLIENTI="./data/clientiOut.txt"
LOGFILECASSE="./data/casseOut.txt"

#Formattazione testo
echo ""

#Calcolo numero righe e scrittura risultati
#Fonte: forum.html.it/forum/showthread/t-1042954.html
RIGHECLIENTI=$(wc -l $LOGFILECLIENTI | awk '{print $1}')
riga=1
echo "Clienti:"
while [ $riga -lt $RIGHECLIENTI ]; do
riga=$((riga + 1))
current=$(head -$riga $LOGFILECLIENTI | tail -1)
echo $current
done

#Formattazione testo
echo ""
echo "Casse:"

#Calcolo numero righe e scrittura risultati
#Fonte: forum.html.it/forum/showthread/t-1042954.html
RIGHECASSE=$(wc -l $LOGFILECASSE | awk '{print $1}')
riga=1
while [ $riga -lt $RIGHECASSE ]; do
riga=$((riga + 1))
current=$(head -$riga $LOGFILECASSE | tail -1)
echo $current
done

#Formattazione testo
echo ""
echo "\e[43m\e[30m           ->  Finita  Analisi <-         \e[49m\e[39m"