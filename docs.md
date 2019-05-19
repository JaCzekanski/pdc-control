All messages:

Response data:
Header:
data[0] = 0xFF 
data[1] = 0x55

ptr = 0
do {
    checksum0 = data[ptr+8];
    checksum1 = data[ptr+9];
    checksum2 = data[ptr+10];
    ptr += 3
} while (ptr < 0x36);

data[62] = checksum0+checksum1+checksum2
data[63] = calculate_checksum()

data[8] == 0x0A // End of data?



Message1 and Message3 response:
data[11] - selected mode
- 0xa0 - min
- 0xa1 - max
- 0xa2 - rotate
- 0x00 - first (5V)
- ...