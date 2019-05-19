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


Message content (without header)

content[0] = 0xA0  // always?
content[1] - selected mode (see above)
content[2] = 0x88  // ?
content[3] = 0x13  // ?
content[4] = 0x01  // ?
content[5] = mode count 
content[6] = 0x00  // ?
// 4B per mode