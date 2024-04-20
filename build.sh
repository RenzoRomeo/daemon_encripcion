#!/bin/bash

CFLAGS="-Wall -Wextra -g"

gcc main.c monitor.c daemonize.c -o daemon $CFLAGS
gcc decrypt.c -o decrypt $CFLAGS
