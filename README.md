SBSEMWS
=======

Simple Bash Scripting Enabled Multithreaded Web Server

Building
---------------------------
Go to the root of the folder and type:
    
    make

You need to have libpthread installed in order to compile succesfully (installed by default on most linux distros) 

Usage 
--------------------------

You can run webserver process by typping

    ./webserver [portnumber]
    
If no port number is specified 1234 is used.

By default server will display index.html if no other file is specified.

If you want to run bash script on the server you can pass in parameters for script in the address line:

    htttp://yourserveraddress:port/filename.bash?first_parameter?second_parameter
