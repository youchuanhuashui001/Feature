#!/usr/bin/env python3

import serial
import random
import string
import time
import sys
import logging
import logging


if len(sys.argv) < 3:
    sys.exit()


serial_num = sys.argv[1]
baudrate = int(sys.argv[2])

time_array = time.localtime(int(time.time()))
time_str = time.strftime("%Y%m%d%H%M%S", time_array)
log_file = ''.join([time_str, ".log"])

logger = logging.getLogger(__name__)
logger.setLevel(level = logging.INFO)
handler = logging.FileHandler(log_file)
handler.setLevel(logging.INFO)
formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
handler.setFormatter(formatter)
 
console = logging.StreamHandler()
console.setLevel(logging.INFO)
 
logger.addHandler(handler)
logger.addHandler(console)

ser = serial.Serial(serial_num, baudrate)
if not ser.is_open:
    print("ser open failed")
    sys.exit()

cnt = 0
error_cnt = 0

logger.info("start test serial %s baudrate %d" % (serial_num, baudrate))
logger.info("record error log in %s" % (log_file))
while True:
    str_len = random.randint(100, 190)
    ran_str = ''.join([random.choice(string.ascii_letters + string.digits + string.punctuation) for i in range(str_len)])
    ran_str = ran_str.join("\r\n")
    #l = [i for i in range(256)]
    #ran_str = b''.join(map(lambda x:int.to_bytes(x,1,'little'),l))

    print(len(ran_str))

    ser.write(ran_str.encode("utf-8"))
    #ser.write(ran_str)
    read_buf = ser.readline();
    try:
        if read_buf.decode('utf-8') != ran_str:
        #if read_buf != ran_str:
            error_cnt += 1
            localtime = time.time()
            logger.error("error cnt : %d" % error_cnt)
            logger.error("origin len : %d" % len(ran_str))
            logger.error("reply len : %d" % len(read_buf))
            logger.error("origin : %s" % ran_str.encode("utf-8"))
            #logger.error("reply : %s" % read_buf.decode('utf-8'))
            logger.error("reply : %s" % read_buf)
            #ser.write("query\r\n".encode('utf-8'))
            #read_buf = ser.readline();
            #logger.error(read_buf.decode('utf-8'))
        else:
            print("origin len : %d" % len(ran_str))
            print("reply len : %d" % len(read_buf))
            print("origin : %s" % ran_str)
            #print("reply : %s" % read_buf.decode('utf-8'))
            print("reply : %s" % read_buf)
            print("cnt : %d error_cnt : %d" % (cnt, error_cnt))
    except:
            error_cnt += 1
            localtime = time.time()
            logger.error("except")
            logger.error("origin len : %d" % len(ran_str))
            logger.error("reply len : %d" % len(read_buf))
            logger.error("len : %d" % len(read_buf))
            logger.error("error cnt : %d" % error_cnt)
            logger.error("origin : %s" % ran_str)
            logger.error("reply : %s" % read_buf)
            ser.write("query\r\n".encode('utf-8'))
            read_buf = ser.readline();
            #logger.error(read_buf.decode('utf-8'))

    cnt += 1
    time.sleep(0.1)



