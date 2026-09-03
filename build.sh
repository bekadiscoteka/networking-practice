#!/bin/bash
touch errlog
mkdir bin 1> /dev/null 2> /dev/null

echo "building server code..."
gcc serv.c -o bin/serv 2> errlog
if [ $? -gt 0 ]; then
	echo "something went wrong, errlog:"
	cat errlog
	exit 1
fi

echo "buliding client code..."
gcc client.c -o bin/client 2> errlog
if [ $? -gt 0 ]; then
	echo "something went wrong, errlog:"
	cat errlog
	exit 1
fi

echo "successfully built!"


