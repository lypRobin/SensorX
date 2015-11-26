#!/usr/bin/env python

from telnetlib import Telnet
import time

# parameters: ipaddr (string):  ip address
# return value: True: valid ip address
#				False: invalid ip address
def SX_check_ip(ipaddr):
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


def SX_get_status(addr):
	mode = "3"
	sx_ip = ""
	sx_sta_ssid = ""
	sx_sta_hostname = ""
	sx_ap_ip = ""
	ax_ap_ssid = ""
	sx_remote_ip = ""
	sx_serial = ""
	sx_mac = ""
	try:
		tn = Telnet(addr[0], addr[1], 3)
		if not tn.read_until("welcome", 3):
			print "Connect to SensorX failed."
			return None
		tn.write("+++AT STATUS\n")
		lines = tn.read_all().split('\n')
		if "OK" not in lines:
			print "Communicate to SensorX failed."
		for line in lines:
			if "MODE" in line:
				mode = line.split('=')[1]
			if "STA IP" in line:
				sx_ip = line.split('=')[1]
			if "STA SSID" in line:
				sx_sta_ssid = line.split('=')[1]
			if "STA HOSTNAME" in line:
				sx_sta_hostname = line.split('=')[1]
			if "AP IP" in line:
				sx_ap_ip = line.split('=')[1]
			if "AP SSID" in line:
				sx_ap_ssid = line.split('=')[1]
			if "REMOTE" in line:
				sx_remote_ip = [line.split('=')[1].split(',')[0], line.split('=')[2]]  # get ip addr and port
			if "Serial" in line:
				sx_serial = line.split('=')[1]
		tn.close()
		return [mode, sx_ip, sx_sta_ssid, sx_sta_hostname, sx_ap_ip, sx_ap_ssid, sx_remote_ip, sx_serial]
	except:
		print "Connect to SensorX failed."
		return None
		tn.close()


def SX_get_mode(addr):
	s = SX_get_status(addr)
	if s == None
		print "Get SensorX mode failed."
		return
	else:
		return s[0]
 

def SX_get_ip(addr):
	s = SX_get_status(addr)
	if s == None
		print "Get SensorX sta ip failed."
		return
	else:
		return s[1]

def SX_get_sta_ssid(addr):
	s = SX_get_status(addr)
	if s == None
		print "Get SensorX sta ssid failed."
		return
	else:
		return s[2]

def SX_get_sta_hostname(addr):
	s = SX_get_status(addr)
	if s == None
		print "Get SensorX sta hostname failed."
		return
	else:
		return s[3]

def SX_get_ap_ip(addr):
	s = SX_get_status(addr)
	if s == None
		print "Get SensorX ap ip failed."
		return
	else:
		return s[4]

def SX_get_ap_ssid(addr):
	s = SX_get_status(addr)
	if s == None
		print "Get SensorX ap ssid failed."
		return
	else:
		return s[5]

# return a list
def SX_get_remote_ip(addr):
	s = SX_get_status(addr)
	if s == None
		print "Get SensorX remote ip failed."
		return
	else:
		return s[6]

# return serial list in: [baud, data_bits, stop_bit, check_bit]
def SX_get_serial_info(addr):
	s = SX_get_status(addr)
	if s == None
		print "Get SensorX sta ip failed."
		return
	else:
		return s[7].split(' ')

####################  SET SensorX  ##################
# set sta ssid and password
def SX_set_sta(addr, data):
	try:
		tn = Telnet(addr[0], addr[1], 3)
		if not tn.read_until("welcome", 3):
			print "Connect to SensorX failed."
			return False

		ssid = data[0]
		password = data[1]
		tn.write("+++AT STA " + ssid + " " + password + "\n")

		if tn.read_until("OK", 3):
			print "Set STA: " + ssid + ", " + password + " OK"
		else 
			print "Set STA failed."
			return False
		tn.close()
	except:
		print "Connect to SensorX failed."
		return None
		tn.close()


def SX_set_sta_ip(addr, data):
	if not SX_check_ip(data):
		print "Error Set Sta ip: Invalid ip."
		return False
	try:
		tn = Telnet(addr[0], addr[1], 3)
		if not tn.read_until("welcome", 3):
			print "Connect to SensorX failed."
			return False

		tn.write("+++AT STAIP " + data + "\n")

		if tn.read_until("OK", 3):
			print "Set STA IP: " + data + " OK"
		else 
			print "Set STA IP failed."
		tn.close()
	except:
		print "Connect to SensorX failed."
		return False
		tn.close()
	return True


def SX_set_sta_hostname(addr, data):
	if ' ' in data:
		print "Error Set Sta hostname: Invalid name."
		return False
	try:
		tn = Telnet(addr[0], addr[1], 3)
		if not tn.read_until("welcome", 3):
			print "Connect to SensorX failed."
			return False

		tn.write("+++AT STAHOSTNAME " + data + "\n")

		if tn.read_until("OK", 3):
			print "Set STA HOSTNAME: " + data + " OK"
		else 
			print "Set STA HOSTNAME failed."
			return False
		tn.close()
	except:
		print "Connect to SensorX failed."
		return False
		tn.close()
	return True


def SX_set_ap_ssid(addr, data):
	if ' ' in data:
		print "Error Set Sta hostname: Invalid name."
		return False
	try:
		tn = Telnet(addr[0], addr[1], 3)
		if not tn.read_until("welcome", 3):
			print "Connect to SensorX failed."
			return False

		tn.write("+++AT AP " + data + "\n")

		if tn.read_until("OK", 3):
			print "Set AP SSID: " + data + " OK"
		else 
			print "Set AP SSID failed."
			return False
		tn.close()
	except:
		print "Connect to SensorX failed."
		return False
		tn.close()
	return True


def SX_set_name(addr, data):
	if ' ' in data:
		print "Error Set Sta hostname: Invalid name."
		return False
	try:
		tn = Telnet(addr[0], addr[1], 3)
		if not tn.read_until("welcome", 3):
			print "Connect to SensorX failed."
			return False

		tn.write("+++AT AP " + data + "\n")
		if tn.read_until("OK", 3):
			tn.write("+++AT STAHOSTNAME " + data + "\n")
			if tn.read_until("OK", 3):
				print "Set SensorX NAME: " + data + " OK"
			else 
				print "Set SensorX NAME failed."
				return False
		else 
			print "Set SensorX NAME failed."
			return False
		tn.close()
	except:
		print "Connect to SensorX failed."
		tn.close()
		return False
	return True


def SX_set_remote_ip(addr, data):
	if not SX_check_ip(data[0]):
		print "Error Set remote ip: Invalid ip."
		return False
	if int(data[1]) > 65536 || not data[0].isdigit():
		print "Error Set remote ip: Invalid port."
		return False
	try:
		tn = Telnet(addr[0], addr[1], 3)
		if not tn.read_until("welcome", 3):
			print "Connect to SensorX failed."
			return False

		tn.write("+++AT AP " + data + "\n")
		if tn.read_until("OK", 3):
			tn.write("+++AT STAHOSTNAME " + data + "\n")
			if tn.read_until("OK", 3):
				print "Set SensorX NAME: " + data + " OK"
			else 
				print "Set SensorX NAME failed."
				return False
		else 
			print "Set SensorX NAME failed."
			return False
		tn.close()
	except:
		print "Connect to SensorX failed."
		tn.close()
		return False
	return True
 





