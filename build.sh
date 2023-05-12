#!/bin/bash

set -euo pipefail

gcc -Werror -O0 -I$HOME/builds -L$HOME/builds/wdsp -o lms example_lms.c -lwdsp
