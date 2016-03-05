#!/usr/bin/env python
# http://stackoverflow.com/questions/323829/how-to-find-out-if-there-is-data-to-be-read-from-stdin-on-windows-in-python

import sys
import re
from socket import *
import select
import os
import signal
import time

serverName = '127.0.0.1'
MSGLEN = 100

if(len(sys.argv) < 4):
    print "Usage: ./client.py <server_host> <server_port> <command> <filename>"
    sys.exit(0)

def handler(signum, frame):
	clientSocket.close()

def send(client, msg):
	client.send(msg)

def receive(client):
	msg = ''
	try:
		msg = client.recv(500)
	except: pass
	return msg[: len(msg)-1]

# set up host connection
serverPort = int(sys.argv[1]);
clientSocket = socket(AF_INET, SOCK_STREAM)
clientSocket.connect((serverName, serverPort))

# set up sigint handler
signal.signal(signal.SIGINT, handler)

# send message to server
init_msg = "\n".join(sys.argv);
send(clientSocket, init_msg)


#message = receive(clientSocket)
#if message:
#	message = message.rstrip()
#	print "server > " + message


print "Data connection on " + sys.argv[2]
print "Control connection on " + sys.argv[1]

time.sleep(2)

dataSocket = socket(AF_INET, SOCK_STREAM)
dataSocket.connect((serverName, int(sys.argv[2])) )

cmd_opt = sys.argv[3]

if(cmd_opt == '-l'):
    while 1:
        msg = clientSocket.recv(500)
        if not msg:
            break
        print msg
else:
    msg2 = clientSocket.recv(500)
    
    if msg2:
        print msg2 + '\n';
        sys.exit(0)

    file = open(sys.argv[4]+".received", "a")
    while 1:
        msg = dataSocket.recv(500)

        if not msg:
            break

        file.write(msg)

    print "Received: " + sys.argv[4]
    print "Wrote file to: " + sys.argv[4] + ".received"
    file.close()

