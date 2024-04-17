#!/bin/bash

CFLAGS="-Wall -Wextra"

gcc main.c monitor.c daemonize.c -o daemon $CFLAGS
