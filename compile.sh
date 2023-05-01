#!/bin/bash

gcc -o cliente ./cliente.c ./Protocol/chat.pb-c.c -lprotobuf-c;
gcc -o servidor ./servidor.c;