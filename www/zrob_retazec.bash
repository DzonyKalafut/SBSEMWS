#!/bin/bash

cat www/top.html

if [ $# -lt 3 ]
then
    echo " Zadal si málo parametrov!"
else
    retazec=$(sed 's/%20/ /g' <<< $1)
    char_count=`grep -o "$2" <<< "$retazec" | wc -l`
    reversed=`echo $retazec | rev`
    zmeneny=$(sed s/$2/$3/g <<< $retazec)

    echo "<p>Zadal si reťazec: <b>$retazec</b></p>"
    echo "<p>Reťazec <b>$retazec</b> odzadu je: <b>$reversed</b></p>" 
    echo "<p>Mením $char_count výskytov <b>$2</b> na <b>$3</b></p>"
    echo "<p>Výsledný reťazec je: <b>$zmeneny</b></p>"
fi

cat www/bottom.html
