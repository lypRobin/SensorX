#!/usr/bin/env python

from telnetlib import Telnet
import time

class SensorX():

	def __init__(self, addr):

		self.conn = Telnet()
		self.address = addr
		self.isconnected = False

		self.mode = "3"
		self.ip = ""
		self.sta_ssid = ""
		self.sta_hostname = ""
		self.ap_ip = ""
		self.ap_ssid = ""
		self.remote_addr = ""
		self.serial = ""
		self.mac = ""		

	# parameters: ipaddr (string):  ip address
	# return value: True: valid ip address
	#				False: invalid ip address
	def check_address(ipaddr):
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


	def connect(self):
		print "connecting..."
		if not check_address(addr):
			print "[Error]: Invalid SensorX ip address."
		try:
			self.conn = Telnet(self.address[0], self.address[1], 3)
			if not self.conn.read_until("welcome", 3):
				print "[Error]: Connect to SensorX failed."
				return False
			self.isconnected = True
		except:
			print "[Error]: Communicate to SensorX failed."
			self.conn.close()
			self.isconnected = False
			return False
		return True

	def close(self):
		self.conn.close()
		self.conn = None
		self.isconnected = False


	def get_status(self):
		if not self.isconnected:
			print "[Error]: SensorX is not connected."
			return None
		try:
			if not self.conn.read_until("welcome", 3):
				print "[Error]: Connect to SensorX failed."
				return None
			self.conn.write("+++AT STATUS\n")
			lines = self.conn.read_all().split('\n')
			if "OK" not in lines:
				print "[Error]: Communicate to SensorX failed."
			for line in lines:
				if "MODE" in line:
					self.mode = line.split('=')[1]
				if "STA IP" in line:
					self.ip = line.split('=')[1]
				if "STA SSID" in line:
					self.sta_ssid = line.split('=')[1]
				if "STA HOSTNAME" in line:
					self.sta_hostname = line.split('=')[1]
				if "AP IP" in line:
					self.ap_ip = line.split('=')[1]
				if "AP SSID" in line:
					self.ap_ssid = line.split('=')[1]
				if "REMOTE" in line:
					ip = line.split('=')[1].split(',')[0]
					port = line.split('=')[2]
					if "255.255.255.255" in line:
						ip = ""
						port = "11311"
					self.remote_addr = [ip, port]
				if "Serial" in line:
					self.serial = line.split('=')[1]
			return [mode, self.ip, self.sta_ssid, self.sta_hostname, self.ap_ip, self.ap_ssid, self.remote_addr, self.serial]
		except:
			print "[Error]: Communicate to SensorX failed."
			return None


	def get_mode(self):
		s = get_status(addr)
		if s == None:
			print "[Error]: Get SensorX mode failed."
			return
		else:
			return s[0]
	 

	def get_ip(self):
		s = get_status(addr)
		if s == None:
			print "[Error]: Get SensorX sta ip failed."
			return
		else:
			return s[1]

	def get_sta_ssid(self):
		s = get_status(addr)
		if s == None:
			print "[Error]: Get SensorX sta ssid failed."
			return
		else:
			return s[2]

	def get_sta_hostname(self):
		s = get_status(addr)
		if s == None:
			print "[Error]: Get SensorX sta hostname failed."
			return
		else:
			return s[3]

	def get_ap_ip(self):
		s = get_status(addr)
		if s == None:
			print "[Error]: Get SensorX ap ip failed."
			return
		else:
			return s[4]

	def get_ap_ssid(self):
		s = get_status(addr)
		if s == None:
			print "[Error]: Get SensorX ap ssid failed."
			return
		else:
			return s[5]

	# return a list
	def get_remote_addr(self):
		s = get_status(addr)
		if s == None:
			print "[Error]: Get SensorX remote ip failed."
			return
		else:
			return s[6]

	# return serial list in: [baud, data_bits, stop_bit, check_bit]
	def get_serial_info(self):
		s = get_status(addr)
		if s == None:
			print "[Error]: Get SensorX sta ip failed."
			return 
		else:
			return s[7].split(' ')

	####################  SET SensorX  ##################
	# set sta ssid and password
	def set_sta(self, data):
		if not self.isconnected:
			print "[Error]: SensorX is not connected."
			return False
		try:
			ssid = data[0]
			password = data[1]
			self.conn.write("+++AT STA " + ssid + " " + password + "\n")

			if not self.conn.read_until("OK", 3): 
				print "[Error]: Set STA failed."
				return False
		except:
			print "[Error]: Communicate to SensorX failed."
			return False
		return True


	def set_sta_ip(self, data):
		if not self.isconnected:
			print "[Error]: SensorX is not connected."
			return False
		if not check_ip(data):
			print "[Error]: Set Sta ip: Invalid ip."
			return False
		try:
			self.conn = Telnet(addr[0], addr[1], 3)
			if not self.conn.read_until("welcome", 3):
				print "[Error]: Connect to SensorX failed."
				return False

			self.conn.write("+++AT STAIP " + data + "\n")

			if not self.conn.read_until("OK", 3):
				print "[Error]: Set STA IP failed."
		except:
			print "[Error]: Communicate to SensorX failed."
			return False
		return True


	def set_sta_hostname(self, data):
		if not self.isconnected:
			print "[Error]: SensorX is not connected."
			return False
		if ' ' in data:
			print "[Error]: Set Sta hostname: Invalid name."
			return False
		try:
			if not self.conn.read_until("welcome", 3):
				print "[Error]: Connect to SensorX failed."
				return False

			self.conn.write("+++AT STAHOSTNAME " + data + "\n")

			if not self.conn.read_until("OK", 3):
				print "[Error]: Set STA HOSTNAME failed."
				return False
		except:
			print "[Error]: Communicate to SensorX failed."
			return False
		return True


	def set_ap_ssid(self, data):
		if not self.isconnected:
			print "[Error]: SensorX is not connected."
			return False
		if ' ' in data:
			print "[Error]: Set Sta hostname: Invalid name."
			return False
		try:
			if not self.conn.read_until("welcome", 3):
				print "[Error]: Connect to SensorX failed."
				return False

			self.conn.write("+++AT AP " + data + "\n")

			if not self.conn.read_until("OK", 3):
				print "[Error]: Set AP SSID failed."
				return False
		except:
			print "[Error]: Communicate to SensorX failed."
			return False
		return True


	def set_name(self, data):
		if not self.isconnected:
			print "[Error]: SensorX is not connected."
			return False
		if ' ' in data:
			print "[Error]: Set Sta hostname: Invalid name."
			return False
		try:
			if not self.conn.read_until("welcome", 3):
				print "[Error]: Connect to SensorX failed."
				return False

			self.conn.write("+++AT AP " + data + "\n")
			if self.conn.read_until("OK", 3):
				self.conn.write("+++AT STAHOSTNAME " + data + "\n")
				if not self.conn.read_until("OK", 3): 
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
		if not self.isconnected:
			print "[Error]: SensorX is not connected."
			return False
		if not check_ip(data[0]):
			print "[Error]: Set remote ip: Invalid ip."
			return False
		if int(data[1]) > 65536 or not data[1].isdigit():
			print "[Error]: Set remote ip: Invalid port."
			return False
		try:
			if not self.conn.read_until("welcome", 3):
				print "[Error]: Connect to SensorX failed."
				return False

			self.conn.write("+++AT AP " + data + "\n")
			if self.conn.read_until("OK", 3):
				self.conn.write("+++AT STAHOSTNAME " + data + "\n")
				if not self.conn.read_until("OK", 3):
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
		if not self.isconnected:
			print "[Error]: SensorX is not connected."
			return False
		try:
			self.conn.write("+++AT GPIO2 2 100")
			if not self.conn.read_until("OK", 3):
				print "[Error]: Reset SensorX failed."
				return False
		except:
			print "[Error]: Communicate to SensorX failed."
			return False
		return True





