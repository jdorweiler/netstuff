#!/usr/bin/env python
# Jason Dorweiler
# cs372 Project 1
#
# Usage: ./ftclient <host> <control port> <option> <file name> <data port>
# Options:  -l (get list of files)
#           -g (get a file)
#
import sys
import re
from socket import *
import select
import os
import signal
import time

MSGLEN = 100

if(len(sys.argv) < 4):
    print "Usage: ./client.py <server_host> <server_port> <filename> <"
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


serverName = sys.argv[1]
# set up host connection
serverPort = int(sys.argv[2]);
clientSocket = socket(AF_INET, SOCK_STREAM)
clientSocket.connect((serverName, serverPort))

# set up sigint handler
signal.signal(signal.SIGINT, handler)

# send message to server
if(len(sys.argv) == 5):
    msg = "\n".join([sys.argv[1], sys.argv[2], sys.argv[4], sys.argv[3]])
    send(clientSocket,msg)

if(len(sys.argv) == 6):
    msg = "\n".join([sys.argv[1], sys.argv[2], sys.argv[5], sys.argv[3], sys.argv[4]])
    send(clientSocket,msg)

time.sleep(1)

cmd_opt = sys.argv[3]
dataSocket = socket(AF_INET, SOCK_STREAM)

if(cmd_opt == '-l'):
    dataSocket.connect((serverName, int(sys.argv[4])) )
    
    while 1:
        msg = clientSocket.recv(500)
        if not msg:
            break
        print msg
else:
    dataSocket.connect((serverName, int(sys.argv[5])) )
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

