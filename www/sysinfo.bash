#!/bin/bash

uptime=`uptime | cut -d' '  -f5-6 | cut -d',' -f1`
filesystems=`df -P | sed 's/$/<br>/' | sed 's/ /\&nbsp;/g'`
memory=`free -h | sed 's/$/<br>/' | sed 's/ /\&nbsp;/g'`

cat www/top.html

echo "<h1>Info o systéme</h1>"
echo "<h2> Všeobecné info:</h2>"
echo "<p>Čas na serveri: <b>`date +%H:%M:%S` CET</b></p>"
echo "<p>Čas behu servera: <b>$uptime</b></p>"
echo "<p>Architektúra procesora: <b>`uname -p`</b></p>"
echo "<p>Jadro systému: <b>`uname -s -r`</b></p>"
echo "<p>Meno servera v sieti: <b>`uname -n`</b></p>"

echo "<h2>Pripojené oddiely disku:</h2>"
echo "$filesystems"

echo "<h2>Využitie pamäte:</h2>"
echo "$memory"

cat www/bottom.html
