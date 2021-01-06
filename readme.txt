1. To run server side scrolltest.c

gcc -export-dynamic -Wall -o filename scrolltest.c $(pkg-config --cflags --libs gtk+-3.0) $(mysql_config --libs --cflags)


2. To run client side one_of_mc.c

gcc -o one one_of_mc.c


3. Lampp should be installed and run in ubuntu

4. To note buslines.html and btnPressed, one_of_mc.c and object one should be in /opt/lampp/htdocs directory

5. Lampp server and apache should be run using sudo /opt/lampp/lampp start

6. In browser user should type localhost/buslines.html 