#!/bin/bash
LOGFILE2="./data/clientiOut.txt"
LOGFILE3="./data/casseOut.txt"
VALGRINDFILE="./data/valgrind.txt"

#Se esiste rimuovo clientOut.txt
if [ -f $LOGFILE2 ] ; then
    rm $LOGFILE2
fi

#Se esiste rimuovo casseOut.txt
if [ -f $LOGFILE3 ] ; then
    rm $LOGFILE3
fi

#Se esiste rimuovo valgrind.txt
if [ -f $VALGRINDFILE ] ; then
    rm $VALGRINDFILE
fi

#Creo i file appena cancellati quindi vuoti
touch $LOGFILE2
touch $LOGFILE3
touch $VALGRINDFILE

#Rendo eseguibile valgrind.txt
chmod a+x $VALGRINDFILE

#Rendo eseguibile test1.txt
chmod +x "./tests/test1.txt"

#Formattazione testo
echo -e "\e[43m\e[30m             INIZIO PROGRAMMA            \e[49m\e[90m"

#Lancio il processo con un timeout di 25secondi eseguendo un sighup e salvando comunque lo status di uscita
timeout --preserve-status -s SIGHUP 25 valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --log-file=$VALGRINDFILE ./exe.o "./tests/test1.txt"
#Aspetto la fine del processo
wait

#Formattazione testo
echo -e "\e[43m\e[30m              FINE PROGRAMMA              \e[49m\e[90m"
echo -e "\e[43m\e[30m   LOG VALGRIND IN $VALGRINDFILE    \e[49m\e[90m"
echo -e ""

#Rendo analisi.sh eseguibile
chmod +x ./analisi.sh
#Eseguo analisi.sh
./analisi.sh
#Aspetto che l'analisi finisca per uscire
wait