#!/bin/bash

CFLAGS="-Wall -Wextra"

gcc main.c watch.c -o daemon $CFLAGS
