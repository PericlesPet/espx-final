#!/bin/bash

rm msgs.txt msgTimes.txt msgs.csv

./bin/espx-final &
ESPXPID=$!
sleep 7200
kill $ESPXPID
