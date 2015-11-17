#!/usr/bin/python
"""A simple program which captures a jpeg from a webcam.
The image is then chopped up and sent in UDP broadcast packets."""
from SimpleCV import Camera
from io import BytesIO
import struct
import binascii
from socket import socket, AF_INET, SOCK_DGRAM, SOL_SOCKET, SO_BROADCAST
import time
import syslog

CAMERA = Camera()

def get_image():
    """Get an image from CAMERA and return a BytesIO object comtaining the
    image as a jpeg."""
    img_buffer = BytesIO()
    try:
        image = CAMERA.getImage()
    except Exception as ex:
        syslog.syslog(syslog.LOG_ERR,
        "Unable to grab image from camera due to %s" % str(ex))
    image.save(img_buffer)
    return img_buffer.getvalue()

def get_image_chunks(byte_str):
    """Get an array of correct-sized chunks of bytes for packetization"""
    byte_strings = []
    count = 0
    mystr = str()
    for img_byte in byte_str:
        mystr += img_byte
        count += 1
        if count == 457:
            byte_strings.append((mystr, count))
            mystr = str()
            count = 0
    byte_strings.append((mystr.ljust(457, '0'), count))
    return byte_strings

def get_packet(last_pkt, seq_num, bytes_sent, payload):
    """build a packet"""
    pkt_struct = struct.Struct("> B l I H 457s")
    try:
        return pkt_struct.pack(last_pkt, binascii.crc32(payload), seq_num,
            bytes_sent, payload)
    except Exception as ex:
        syslog.syslog(syslog.LOG_ERR, "Unable to send packet due to %s" %\
            str(ex))


if __name__ == "__main__":
    SOCK = socket(AF_INET, SOCK_DGRAM, 0)
    SOCK.setsockopt(SOL_SOCKET, SO_BROADCAST, 1)
    SOCK.connect(('255.255.255.255', 31337))
    while True:
        FILE_BUF = get_image()
        BYTE_ARRAYS = get_image_chunks(FILE_BUF)
        COUNT = 0
        for (byte_array, size) in BYTE_ARRAYS:
            last_pkt = 0
            if COUNT == len(BYTE_ARRAYS) - 1:
                last_pkt = 1
            SOCK.send(get_packet(last_pkt, COUNT, size, byte_array))
            COUNT += 1
        time.sleep(1)
