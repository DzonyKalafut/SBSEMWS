#!/bin/bash

cat www/top.html
echo "<h2>Info o systéme</h2>"
uname -a
echo "<h2>Celkový čas behu servera</h2>"
uptime
cat www/bottom.html
