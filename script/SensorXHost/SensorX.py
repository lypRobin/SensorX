#!/usr/bin/env python
#############################################################################
#  File	: SensorX.py
#  SensorX host module that communicate to esp8266 based on SensorX board.
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

from telnetlib import Telnet
import time

class SensorX():

	def __init__(self, addr):

		self._conn = Telnet()
		self._address = addr
		self._isconnected = False

		self._mode = ""
		self._ip = ""
		self._sta_ssid = ""
		self._sta_hostname = ""
		self._ap_ip = ""
		self._ap_ssid = ""
		self._remote_addr = ""
		self._serial = ""
		self._mac = ""		

	# parameters: ipaddr (string):  ip address
	# return value: True: valid ip address
	#				False: invalid ip address
	def check_address(self, ipaddr):
		'''check ip address available.'''

		print "checking ip."
		ips = ipaddr[0].strip().split('.')
		if len(ips) != 4:
			return False
		else:
			for ip in ips:
				if not ip.isdigit():
					return False
				else:
					if int(ip) < 0 or int(ip) > 255:
						return False
		port = ipaddr[1]
		if not port.isdigit() or int(port) > 65535:
			return False
		return True


	def get_status(self):
		print "get status"
		if not self._isconnected:
			print "[Error]: SensorX is not connected."
			return None
		try:
			if not self._conn.read_until("welcome", 3):
				print "[Error]: Connect to SensorX failed."
				return None
			self._conn.write("+++AT STATUS\n")
			lines = self._conn.read_all().split('\n')
			if "OK" not in lines:
				print "[Error]: Communicate to SensorX failed."
			for line in lines:
				if "MODE" in line:
					self._mode = line.split('=')[1]
				if "STA IP" in line:
					self._ip = line.split('=')[1]
				if "STA SSID" in line:
					self._sta_ssid = line.split('=')[1]
				if "STA HOSTNAME" in line:
					self._sta_hostname = line.split('=')[1]
				if "AP IP" in line:
					self._ap_ip = line.split('=')[1]
				if "AP SSID" in line:
					self._ap_ssid = line.split('=')[1]
				if "REMOTE" in line:
					ip = line.split('=')[1].split(',')[0]
					port = line.split('=')[2]
					if "255.255.255.255" in line:
						ip = ""
						port = "11311"
					self._remote_addr = [ip, port]
				if "Serial" in line:
					self._serial = line.split('=')[1]
			return [mode, self._ip, self._sta_ssid, self._sta_hostname, self._ap_ip, self._ap_ssid, self._remote_addr, self._serial]
		except:
			print "[Error]: Communicate to SensorX failed."
			return None

	def connect(self):
		print "connecting..."
		if not self.check_address(self._address):
			print "[Error]: Invalid SensorX ip address."
		try:
			self._conn = Telnet(self._address[0], self._address[1], 3)
			if not self._conn.read_until("welcome", 3):
				print "[Error]: Connect to SensorX failed."
				return False
			self._isconnected = True
		except:
			print "[Error]: Communicate to SensorX failed."
			self._conn.close()
			self._isconnected = False
			return False
		return True

	def close(self):
		self._conn.close()
		self._conn = None
		self._isconnected = False


	def is_connected():
		if self._isconnected:
			return True
		else:
			return False


	def get_mode(self):
		return self._mode
	 
	def get_ip(self):
		return self._ip

	def get_sta_ssid(self):
		return self._sta_ssid

	def get_sta_hostname(self):
		return self._sta_hostname

	def get_ap_ip(self):
		return self._ap_ip

	def get_ap_ssid(self):
		return self._ap_ssid

	# return a list
	def get_remote_addr(self):
		return self._remote_addr

	# return serial list in: [baud, data_bits, stop_bit, check_bit]
	def get_serial_info(self):
		return self._serial

	####################  SET SensorX  ##################
	# set sta ssid and password
	def set_sta(self, data):
		if not self._isconnected:
			print "[Error]: SensorX is not connected."
			return False
		try:
			ssid = data[0]
			password = data[1]
			self._conn.write("+++AT STA " + ssid + " " + password + "\n")

			if not self._conn.read_until("OK", 3): 
				print "[Error]: Set STA failed."
				return False
		except:
			print "[Error]: Communicate to SensorX failed."
			return False
		return True


	def set_sta_ip(self, data):
		if not self._isconnected:
			print "[Error]: SensorX is not connected."
			return False
		if not check_ip(data):
			print "[Error]: Set Sta ip: Invalid ip."
			return False
		try:
			self._conn = Telnet(addr[0], addr[1], 3)
			if not self._conn.read_until("welcome", 3):
				print "[Error]: Connect to SensorX failed."
				return False

			self._conn.write("+++AT STAIP " + data + "\n")

			if not self._conn.read_until("OK", 3):
				print "[Error]: Set STA IP failed."
		except:
			print "[Error]: Communicate to SensorX failed."
			return False
		return True


	def set_sta_hostname(self, data):
		if not self._isconnected:
			print "[Error]: SensorX is not connected."
			return False
		if ' ' in data:
			print "[Error]: Set Sta hostname: Invalid name."
			return False
		try:
			if not self._conn.read_until("welcome", 3):
				print "[Error]: Connect to SensorX failed."
				return False

			self._conn.write("+++AT STAHOSTNAME " + data + "\n")

			if not self._conn.read_until("OK", 3):
				print "[Error]: Set STA HOSTNAME failed."
				return False
		except:
			print "[Error]: Communicate to SensorX failed."
			return False
		return True


	def set_ap_ssid(self, data):
		if not self._isconnected:
			print "[Error]: SensorX is not connected."
			return False
		if ' ' in data:
			print "[Error]: Set Sta hostname: Invalid name."
			return False
		try:
			if not self._conn.read_until("welcome", 3):
				print "[Error]: Connect to SensorX failed."
				return False

			self._conn.write("+++AT AP " + data + "\n")

			if not self._conn.read_until("OK", 3):
				print "[Error]: Set AP SSID failed."
				return False
		except:
			print "[Error]: Communicate to SensorX failed."
			return False
		return True


	def set_name(self, data):
		if not self._isconnected:
			print "[Error]: SensorX is not connected."
			return False
		if ' ' in data:
			print "[Error]: Set Sta hostname: Invalid name."
			return False
		try:
			if not self._conn.read_until("welcome", 3):
				print "[Error]: Connect to SensorX failed."
				return False

			self._conn.write("+++AT AP " + data + "\n")
			if self._conn.read_until("OK", 3):
				self._conn.write("+++AT STAHOSTNAME " + data + "\n")
				if not self._conn.read_until("OK", 3): 
					print "[Error]: Set SensorX NAME failed."
					return False
			else:
				print "[Error]: Set SensorX NAME failed."
				return False
		except:
			print "[Error]: Communicate to SensorX failed."
			return False
		return True


	def set_remote_addr(self, data):
		if not self._isconnected:
			print "[Error]: SensorX is not connected."
			return False
		if not check_ip(data[0]):
			print "[Error]: Set remote ip: Invalid ip."
			return False
		if int(data[1]) > 65536 or not data[1].isdigit():
			print "[Error]: Set remote ip: Invalid port."
			return False
		try:
			if not self._conn.read_until("welcome", 3):
				print "[Error]: Connect to SensorX failed."
				return False

			self._conn.write("+++AT AP " + data + "\n")
			if self._conn.read_until("OK", 3):
				self._conn.write("+++AT STAHOSTNAME " + data + "\n")
				if not self._conn.read_until("OK", 3):
					print "[Error]: Set SensorX NAME failed."
					return False
			else:
				print "[Error]: Set SensorX NAME failed."
				return False
		except:
			print "[Error]: Communicate to SensorX failed."
			return False
		return True


	def reset_sensorx(self):
		if not self._isconnected:
			print "[Error]: SensorX is not connected."
			return False
		try:
			self._conn.write("+++AT GPIO2 2 100")  #  reset arduino
			if not self._conn.read_until("OK", 3):
				print "[Error]: Reset SensorX failed."
				return False

		except:
			print "[Error]: Communicate to SensorX failed."
			return False
		return True


	def reset_sensorx(self):
		if not self._isconnected:
			print "[Error]: SensorX is not connected."
			return False
		try:
			self._conn.write("+++AT RESET")  #reset esp8266
			if not self._conn.read_until("OK", 3):
				print "[Error]: Reset SensorX failed."
				return False
		except:
			print "[Error]: Communicate to SensorX failed."
			return False
		return True





