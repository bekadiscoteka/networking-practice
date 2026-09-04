#!/bin/bash
mkdir bin 1> /dev/null 2> /dev/null

case $1 in
	"tcp")
		echo "TCP selected"
		from="tcp/client.c"
		to="tcp/serv.c"
		frombin="bin/client"
		tobin="bin/serv"
		;;
	"udp")
		echo "UDP selected"
		from="udp/talkto.c"
		to="udp/listen.c"
		frombin="bin/talkto"
		tobin="bin/listen"
		;;
	*)
		echo "choose the socket type"
		exit 1
		;;

esac

echo "building $to code..."
gcc $to -o $tobin 2> err.log
if [ $? -gt 0 ]; then
	echo "something went wrong, errlog:"
	cat err.log
	exit 1
fi

echo "buliding $from code..."
gcc $from -o $frombin 2> err.log
if [ $? -gt 0 ]; then
	echo "something went wrong, errlog:"
	cat err.log
	exit 1
fi

echo "successfully built!"


