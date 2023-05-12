#!/bin/bash

set -xeuo pipefail

gcc -std=gnu99  -Wall -O3 -I$HOME/builds -L$HOME/builds/wdsp -o kiwi_lms example_kiwi_lms.c -lm
