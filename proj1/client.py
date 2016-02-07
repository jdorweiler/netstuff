#!/usr/bin/env python
# used this source to figure out how to read stdin
# http://stackoverflow.com/questions/323829/how-to-find-out-if-there-is-data-to-be-read-from-stdin-on-windows-in-python

import sys
import re
from socket import *
import select
import os
import signal

serverName = sys.argv[1];
serverPort = int(sys.argv[2]);
clientSocket = socket(AF_INET, SOCK_STREAM)
clientSocket.connect((serverName, serverPort))

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

signal.signal(signal.SIGINT, handler)

message = receive(clientSocket)
if message:
	message = message.rstrip()
	print "server > " + message

name = sys.stdin.readline().rstrip();
send(clientSocket, "Connection from "+ name + "\n")

quit = re.compile("\quit");

while 1:

	message = receive(clientSocket)
	if message:
		print "server > " + message

	print "you > ";
	toSend = sys.stdin.readline();
	sys.stdin.flush()

	if toSend:
		if( toSend.encode("hex") == "5c717569740a" ):
			print "Closing Connection"
			clientSocket.close()
			break
		toSend = name+"> "+toSend;
		send(clientSocket, toSend)
		sys.stdout.flush()
