#!/usr/bin/python
"""listen on port 31337 for image packets"""
from socket import socket, AF_INET, SOCK_DGRAM
import struct
import time
from io import BytesIO
import binascii
import syslog
SOCK = socket(AF_INET, SOCK_DGRAM)
HOST = '255.255.255.255'
PORT = 31337
PKT_STRUCT = struct.Struct("> B l I H 457s")
try:
    SOCK.bind((HOST, PORT))
except Exception as ex:
    syslog.syslog(syslog.LOG_ERR, "Unable to bind to socket due to %s" % str(ex))
BAD_PACKET = False
next_packet = 0
while True:
    (last_pkt, CRC_32, seq_num, bytes_sent, payload) =\
        PKT_STRUCT.unpack(SOCK.recvfrom(512)[0])
    if seq_num == 0:
        BAD_PACKET = False
        img_buffer = BytesIO()
    if next_packet != seq_num:
        BAD_PACKET = True
        syslog.syslog(syslog.LOG_DEBUG, "Missed packet sequence number")
    if binascii.crc32(payload) != CRC_32:
        BAD_PACKET = True
        syslog.syslog(syslog.LOG_DEBUG, "Bad payload CRC in packet")
    if BAD_PACKET == False:
        for byte in payload:
            img_buffer.write(byte)
        next_packet += 1
    if last_pkt == True:
        next_packet = 0
        file_name = str(int(time.time())) + '.jpg'
        jpg_file = open(file_name, 'wb')
        jpg_file.write(img_buffer.getvalue())
        jpg_file.close()
