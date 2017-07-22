#!/bin/bash
cat `cmake . | grep -- "-- Source_Files:" | awk '{print $3}' | sed  's/;/ /g'` > all_in_one_pre.c
cpp all_in_one_pre.c -I pth -I . -I core/external/github/petewarden/c_hashmap/ `cmake . | grep -- "-- Compile_Definitions:" | awk '{print $3}' | sed 's/\(^\|;\)/ -D/g'`> all_in_one.c
echo "gcc "` cmake . | grep -- "-- Link_Options:" | awk '{print $3}' | sed 's/-l//g' | sed 's/\(^\|;\)/ -l/g'`" all_in_one.c"

#cat `cmake . | grep -- "-- Source_Files:" | awk '{print $3}' | sed  's/;/ /g'` > all_in_one_pre.c
#cpp all_in_one_pre.c -I . -I core/external/github/petewarden/c_hashmap/ `cmake . | grep -- "-- Compile_Definitions:" | awk '{print $3}' | sed 's/\(^\|;\)/ -D/g'`> all_in_one.c
#echo "gcc "` cmake . | grep -- "-- Link_Options:" | awk '{print $3}' | sed 's/-l//g' | sed 's/\(^\|;\)/ -l/g'`" all_in_one.c"

