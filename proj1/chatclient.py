#!/usr/bin/env python
# Jason Dorweiler
# cs327 Project 1
# 
# Desc: Start a chat client on <hostname> <port>
# Usage: python chatclient.py localhost 5001
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

# wI was going to try and set up some sigint and sigpipe
# handlers to have it exit a little more gracefully but 
# didn't have time :()
signal.signal(signal.SIGINT, handler)

# the initial startup message sent from the server asking
# for your name
message = receive(clientSocket)
if message:
	message = message.rstrip()
	print "server > " + message

# send the name back to the server
name = sys.stdin.readline().rstrip();
send(clientSocket, "Connection from "+ name + "\n")

# regex to check for the quit option
quit = re.compile("\quit");

while 1:

	# receive the message
	message = receive(clientSocket)
	if message:
		print "server > " + message

	# Read the message from the client
	print "you > ";
	toSend = sys.stdin.readline();
	sys.stdin.flush()

	# check for the quit message.  In hex because I got sick of the 
	# regex not working because of newlines and nonprinting chars
	if toSend:
		if( toSend.encode("hex") == "5c717569740a" ):
			print "Closing Connection"
			clientSocket.close()
			break

		# send the mesage to the server
		toSend = name+"> "+toSend;
		send(clientSocket, toSend)
		sys.stdout.flush()
