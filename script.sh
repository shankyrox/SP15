#!/bin/bash
n=1
while (( $n <= 1000))
do
	echo "starting client number : $n"
     client 127.0.0.1 9006 &
	n=$(( n+1 ))	
done
