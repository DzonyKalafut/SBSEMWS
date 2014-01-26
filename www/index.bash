#!/bin/bash

rnd=$[ ($RANDOM % 4) + 1 ]

cat www/top.html

if [ $1 ]
then
    retazec=$(sed 's/%20/ /g' <<< $1) 
    if [ $3 ]
    then
        echo "<h1 id=\"nadpis\" style=\"color: $3;\">$retazec</h1>"
    else
        echo "<h1 id=\"nadpis\">$retazec</h1>"
    fi
else
    echo "<h1 id=\"nadpis\">Vitajte!!!</h1>"
fi

if  [ $2 ]
then 
    if [ $2 -le 4 ]
    then
        echo "<p align="center"><img alt=\"Obraztok neni tu\" src=\"images/lama$2.jpg\"></p>"
    else
        echo "<p align="center"><img alt=\"Obraztok neni tu\" src=\"images/lama$rnd.jpg\"></p>"
    fi
else
    echo "<p align="center"><img alt=\"Obraztok neni tu\" src=\"images/lama$rnd.jpg\"></p>"
fi

cat www/bottom.html
