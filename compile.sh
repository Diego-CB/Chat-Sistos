#!/bin/bash

gcc -o client ./clie.c ./Protocol/chat.pb-c.c -lprotobuf-c;
gcc -o server ./servidor.c ./Protocol/chat.pb-c.c -lprotobuf-c -lpthread;