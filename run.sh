#!/bin/bash

code="webserver.c"
port=1234

# Vypisane udajov
echo "//////////////////////////////////////////////////////////"
echo "$(tput bold)Ján Hajnár"
echo "Simple multithreaded webserver with bash scripting support $(tput sgr0)"
echo "//////////////////////////////////////////////////////////"

# Zobrazenie dokumentacie
while true; do
    echo -e "\nDo you want to see program documentation? [Y/n]"
    read -n 1 -s  odpoved
    case $odpoved in
        [Yy]* | "" ) 
            echo ""; less $doc; break;;
        [Nn]* ) 
            echo ""; break;;
        * ) 
            echo -e "Please answer Yes(y) or No(n)";;
    esac
done

# Preklad programu
echo -e "Build start..."
make

# Zistim ci kompilacia prebehla uspesne
if [ $? -ne 0 ]
then
    echo "$(tput setaf 1)$(tput bold)Build finished with errors , please check source code!!!$(tput sgr0)"
    exit 1
else
    echo "$(tput setaf 2)$(tput bold)Build succesfull$(tput sgr0)"
fi


# Spustenie programu a mozost ukoncenia programu stlacenim klavesy Q
echo -e "\nStarting server... (Functionality of the server can be tested via webbrowser)"
echo -e "You can terminate webserver process at any time by pressing 'Q'"
./webserver $port &

while ps -p $! > /dev/null 
do
    read -n 1 -s -t 1 odpoved
    case $odpoved in
        [Qq]* ) 
            echo -e "\nTerminating webserver process..."
            killall webserver
            break;;
    esac
done

wait $!

# Zistim exit status programu
if [ $? -ne 0 ]
then
    echo "$(tput setaf 1)$(tput bold)Webserver process ended with error$(tput sgr0)"
    exit 1
else
    echo "$(tput setaf 2)$(tput bold)Webserver process ended normally$(tput sgr0)"
fi 

# Zobrazenie zdrojoveho kodu programu
while true; do
    echo -e "\nDo you wish to see webserver source code? [A/n]"
    read -n 1 -s  odpoved
    case $odpoved in
        [Yy]* | "" ) 
            echo ""; less $code; break;;
        [Nn]* ) 
            echo ""; break;;
        * ) 
            echo -e "Please answer Yes(y) or No(n)";;
    esac
done

echo "Bye :)"
exit 0
