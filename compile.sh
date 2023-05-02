#!/bin/bash

gcc -o cliente ./clie.c ./Protocol/chat.pb-c.c -lprotobuf-c;
gcc -o servidor ./servidor.c ./Protocol/chat.pb-c.c -lprotobuf-c -lpthreads;