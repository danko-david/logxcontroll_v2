#!/bin/bash
cat `cmake . | grep -- "-- Source_Files:" | awk '{print $3}' | sed  's/;/ /g'` |
cpp -I . -I core/external/github/petewarden/c_hashmap/ `cmake . | grep -- "-- Compile_Definitions:" | awk '{print $3}' | sed 's/\(^\|;\)/ -D/g'`> all_in_one.c
echo "gcc "` cmake . | grep -- "-- Link_Options:" | awk '{print $3}' | sed 's/-l//g' | sed 's/\(^\|;\)/ -l/g'`" all_in_one.c"
