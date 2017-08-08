#!/bin/bash
./logxcontroll-core novaprova --list | grep -v "^np:" | sed "s/^\(\)\(.*\)/add_test\(\2 logxcontroll-core novaprova repository.\2\)/"
