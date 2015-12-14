#!/usr/bin/env python
#############################################################################
#  File	: SXServer.py
#  SensorX host server.
#  Copyright (C) 2015 - 2016, Yanpeng Li <lyp40293@gmail.com>
# 
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of version 3 of the GNU General Public License as
#  published by the Free Software Foundation.
#  
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
#  GNU General Public License for more details.
# 
#  You should have received a copy of the GNU General Public License along
#  with this program.	If not, see <http://www.gnu.org/licenses/>.
#############################################################################

from SocketServer import ThreadingTCPServer, StreamRequestHandler
import traceback
import socket

class MyThreadingTCPServer(ThreadingTCPServer):
	def server_bind(self):
		self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
		self.socket.bind(self.server_address)

class HostServerHandler(StreamRequestHandler):
	def handle(self):
		while True:
			data = self.rfile.readline().strip()
			print "Receive from %s:" % self.client_address[0]
			print "Data is: %s" % data
			# pos = "POST" in data
			# if pos:
			self.wfile.write("+++POST")
			if not data:
				print "No data..."
				break


if __name__ == '__main__':
	host = ""
	port = 11311
	addr = (host, port)
	server = MyThreadingTCPServer(addr, HostServerHandler)
	server.serve_forever()

# import SocketServer

# class MyTCPHandler(SocketServer.StreamRequestHandler):

#     def handle(self):
#         # self.rfile is a file-like object created by the handler;
#         # we can now use e.g. readline() instead of raw recv() calls
#         self.data = self.rfile.readline().strip()
#         print "{} wrote:".format(self.client_address[0])
#         print self.data
#         # Likewise, self.wfile is a file-like object used to write back
#         # to the client
#         self.wfile.write(self.data.upper())

# if __name__ == "__main__":
#     HOST, PORT = "localhost", 999

#     # Create the server, binding to localhost on port 9999
#     server = SocketServer.TCPServer((HOST, PORT), MyTCPHandler)

#     # Activate the server; this will keep running until you
#     # interrupt the program with Ctrl-C
#     server.serve_forever()





