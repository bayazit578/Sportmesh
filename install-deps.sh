#!/bin/bash

git clone https://github.com/nopnop2002/esp-idf-sx126x.git .deps
mkdir -p components/ra01s
cp -r .deps/components/ra01s ./components/ra01s
