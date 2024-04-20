#!/bin/bash

DAEMON_NAME="daemon"
CFLAGS="-Wall -Wextra -g"

gcc main.c monitor.c daemonize.c -o $DAEMON_NAME $CFLAGS
gcc decrypt.c -o decrypt $CFLAGS
