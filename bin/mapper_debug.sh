#!/bin/sh

echo
echo "Starting ester server and client and attaching gdb to mapper process:"
echo

./ester --margs --debug &
SERVER_PID=$!
sleep 2

./example &
CLIENT_PID=$!
sleep 2


MPID=$(pgrep -u $USER -f "mapper --debug" | sort -n | head -n 1)
echo "Mapper PID is $MPID"

CMDFILE=tempfile
echo "
list
set attached=true
" > $CMDFILE

gdb -f -x $CMDFILE p $MPID

rm $CMDFILE

kill $CLIENT_PID $SERVER_PID

