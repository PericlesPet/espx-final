#!/bin/bash

for i in 6666 7777 8896; do
    sed "s/_/,/g;" msgs$i.txt > msgs$i.csv
done

# https://unix.stackexchange.com/questions/99350/how-to-insert-text-before-the-first-line-of-a-file

