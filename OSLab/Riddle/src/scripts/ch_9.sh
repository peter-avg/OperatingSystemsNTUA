#!/bin/bash

for i in {0..10};
do
    truncate -s 1073741824 bf0$i;
    echo 0 >> bf0$i;
done


strace ~/Downloads/riddle-20231009-0/riddle;

rm bf*;
