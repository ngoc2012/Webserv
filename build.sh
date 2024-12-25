#!/bin/sh

make re && make clean

cp ./webserv ../Resume/webserv/conf/

./webserv .3.conf
