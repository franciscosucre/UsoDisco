#!/bin/bash
make clean
# scp [-r] <file(s)> <username>@<host>:<destinyDirectory>
scp -r ./Makefile ./messages.c ./messages.h ./usoDiscoMain.c 10-10717@sheeana.ldc.usb.ve:/preg/10-10717/UsoDisco_Operativos_1
