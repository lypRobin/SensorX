#!/usr/bin/env python
#############################################################################
#  File	: wifi_uploader.py
#  Automatic uploader to SensorX(Arduino) wirelessly.
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

import sys
from telnetlib import Telnet 
import subprocess
import time

USAGE = 'Usage: wifi_uploader.py host hex_directory [port]'

# parameters: host (string): ip host address
#			  port (int): port
#			  hex_dir (string): arduino hex file directory
def avr_uploader(host, port, hex_dir):
	"""avrdude arduino uploader using commandline. """

	MSG = {'0': 'Upload successfully.', \
		   '1': 'Upload error: Invalid HEX file directory.', \
		   '2': 'Upload error: Sync error.'}

	port = '%d' % port
	cmd = 'avrdude -v -c avrisp -p m328p -P net:' + host + ':' + port + ' -F -U flash:w:' + hex_dir + ':i'
	print "Uploading......"
	try:
		p = subprocess.Popen(cmd, shell=True, stdout = subprocess.PIPE, stderr= subprocess.STDOUT)
		res = '0'
		for line in p.stdout.readlines():
			print line
			if 'bytes of flash verified' in line:
				res = '0'
			if 'No such file or directory' in line:
				res = '1'
			if 'programmer is not responding' in line:
				res = '2'
		p.wait()

	except:
		print "AVR command error."
		return
	
	print MSG[res]

# parameters: ipaddr (string):  ip address
# return value: True: valid ip address
#				False: invalid ip address
def check_ip(ipaddr):
	'''check ip address available.'''
	ips = ipaddr.strip().split('.')
	if len(ips) != 4:
		return False
	else:
		for ip in ips:
			if not ip.isdigit():
				print "is not digit"
				return False
			else:
				if int(ip) < 0 or int(ip) > 255:
					print "ip invalid: " + ip 
					return False
	return True

# parameters: host (string): ip host address
#			  port (int): port
#			  hex_dir (string): arduino hex file directory
def uploader(host, hex_dir, port=23):
	if not check_ip(host):
		print "Invalid host ip address: " + host
		print USAGE
		return
	if port < 0 or port > 65535:
		print "Invalid port."
		print USAGE
		return

	try:
		print "Connecting to SensorX: " + host + "..."
		tn = Telnet(host, port, 5)
		# tn.set_debuglevel(10)
		print "Connected."
		tn.write("+++AT\n")
		if tn.read_until("OK", 3):
			print "Communication OK."
			tn.write("+++AT GPIO2 2 100")
			time.sleep(0.4)
			print "Reset SensorX OK."
			avr_uploader(host, port, hex_dir)
		else:
			print "Communication ERROR."
			return

		tn.close()
	except:
		print "Cannot connect to host: " + host
		return


# main
if __name__ == '__main__':
	argc = len(sys.argv)

	if argc < 3 or argc > 4:
		print USAGE
		sys.exit()
	if sys.argv[1] == '-h' or sys.argv[1] == '--help':
		print USAGE

	HOST = sys.argv[1]

	# host and hex directory
	if argc == 3: 
		HEX_DIR = sys.argv[2]
		uploader(HOST, HEX_DIR)
	# host, port and hex directory
	else:
		HEX_DIR = sys.argv[2]
		try:
			PORT = int(sys.argv[3])
			uploader(HOST, HEX_DIR, PORT)
		except:
			print "Invalid port."
			print USAGE
			sys.exit()
		
