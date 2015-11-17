#!/usr/bin/python
"""listen on port 31337 for image packets"""
from socket import socket, AF_INET, SOCK_DGRAM
import struct
import time
from io import BytesIO
import binascii
import syslog
import Image
from cStringIO import StringIO
import psutil
SOCK = socket(AF_INET, SOCK_DGRAM)
HOST = '255.255.255.255'
PORT = 31337
PKT_STRUCT = struct.Struct("> B l I H 457s")
try:
    SOCK.bind((HOST, PORT))
except Exception as ex:
    syslog.syslog(syslog.LOG_ERR, "Unable to bind to socket due to %s" %\
    str(ex))
BAD_PACKET = False
NEXT_PACKET = 0
while True:
    (LAST_PKT, CRC_32, SEQ_NUM, bytes_sent, PAYLOAD) =\
        PKT_STRUCT.unpack(SOCK.recvfrom(512)[0])
    if SEQ_NUM == 0:
        BAD_PACKET = False
        IMG_BUFFER = BytesIO()
    if NEXT_PACKET != SEQ_NUM:
        BAD_PACKET = True
        syslog.syslog(syslog.LOG_DEBUG, "Missed packet sequence number")
    if binascii.crc32(PAYLOAD) != CRC_32:
        BAD_PACKET = True
        syslog.syslog(syslog.LOG_DEBUG, "Bad PAYLOAD CRC in packet")
    if BAD_PACKET == False:
        for i in range(0, bytes_sent):
            IMG_BUFFER.write(list(PAYLOAD)[i])
        NEXT_PACKET += 1
    if LAST_PKT == True:
        for proc in psutil.process_iter():
            if proc.name == "display":
                proc.kill()
        NEXT_PACKET = 0
        FILE_NAME = str(int(time.time())) + '.jpg'
        JPG_FILE = open(FILE_NAME, 'wb')
        JPG_FILE.write(IMG_BUFFER.getvalue())
        JPG_FILE.close()
        Image.open(StringIO(IMG_BUFFER.getvalue())).show()
