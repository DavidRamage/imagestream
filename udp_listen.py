#!/usr/bin/python
"""listen on port 31337 for image packets"""
from socket import socket, AF_INET, SOCK_DGRAM
import struct

sock = socket(AF_INET, SOCK_DGRAM)
HOST = '255.255.255.255'
PORT = 31337
PKT_STRUCT = struct.Struct("> B l I H 457s")
sock.bind((HOST, PORT))
file_out = open("udp_rx.jpg", "wb")
while True:
    (last_pkt, crc_32, seq_num, bytes_sent, payload) =\
        PKT_STRUCT.unpack(sock.recvfrom(512)[0])
    for byte in payload:
        file_out.write(byte)
    if last_pkt == 1:
        file_out.close()
        exit()
