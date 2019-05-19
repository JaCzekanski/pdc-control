#include <hidapi.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>

const int BUF_SIZE = 255;

// Init/ get info
uint8_t msg1[65] =  "\xff\x55\x21\xfc\xdc\x4f\x08\xa2\x03\x00\x00\x00\x00\x00\x00\x00" \
                    "\xa4\x00\xa6\x00\x06\xcd\x00\x10\xc0\xf3\x1b\x00\x4d\x13\x00\x10" \
                    "\x06\xcd\x00\x10\x00\x00\x00\xc0\x00\x00\x00\x00\x00\x00\x00\x00" \
                    "\x03\x00\x00\x00\x00\x00\x00\x40\x00\x00\x00\x00\xf8\xcc\x18\x5e";

uint8_t resp1[65] = "\xff\x55\x3e\x4a\x15\x6c\x4a\x07\x02\x00\x00\x00\x00\x00\x00\x00" \
                    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
                    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
                    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\xb0";

// Get device name
uint8_t msg2[65] =  "\xff\x55\x21\xfc\xdc\x4f\x08\xa2\x0a\x05\x00\x38\x00\x08\x0f\x00" \
                    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
                    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
                    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x5e\xa4";

uint8_t resp2[65] = "\xff\x55\x3e\x4e\x15\x6c\x4e\x07\x0a\x0f\x50\x44\x43\x30\x30\x31" \
                    "\x5f\x50\x43\x43\x46\x47\x31\x2e\x30\x00\x00\x00\x00\x00\x00\x00" \
                    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
                    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xd2\x88";

// Get info?
uint8_t msg3[65] =  "\xff\x55\x21\xfc\xdc\x4f\x08\xa2\x0a\x05\x00\xfc\x00\x08\x34\x00" \
                    "\xc0\xf3\x1b\x00\x38\xf4\x1b\x00\x00\xf9\x5e\x02\xa3\x19\xd2\x00" \
                    "\x98\x35\xd6\x00\xa4\xf8\x5e\x02\x06\x3e\xd2\x00\xf8\xf8\x5e\x02" \
                    "\x98\x35\xd6\x00\x50\x00\x00\x00\x44\x00\x00\x00\x43\x00\xc2\x08";

uint8_t resp3[65] = "\xff\x55\x3e\x51\x15\x6c\x51\x08\x0a\x34\xa0\xa0\x88\x13\x01\x01" \
                    "\x00\x5a\x90\x01\x26\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
                    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x61\xff\xff" \
                    "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xe8\x66\x23";

void doThing(hid_device* handle, uint8_t buffer[65]) {
    uint8_t data[65];
    data[0] = 0;

    memcpy(data + 1, buffer, 64);
    hid_write(handle, data, 65);

    hid_read(handle, data, 65);
    for (int y = 0; y<4; y++) {
        for (int x = 0; x< 16; x++) {
            printf("%02X ", data[y*16 + x]);
        }
        printf("\n");
    }
}

void getFirmwareName(hid_device* handle) {
    uint8_t data[65];
    data[0] = 0;

    memcpy(data + 1, msg2, 64);
    hid_write(handle, data, 65);

    hid_read(handle, data, 65);

    if (!(data[0] == 0xff && data[1] == 0x55)) {
        printf("Invalid header response\n");
    }

    uint8_t length = data[9];
    printf("Firmware: ");
    for (int i = 0; i< length; i++) {
        printf("%c", data[10+i]);
    }
    printf("\n");
}

void readModes(hid_device* handle) {
    uint8_t data[65];
    data[0] = 0;

    memcpy(data + 1, msg3, 64);
    hid_write(handle, data, 65);

    hid_read(handle, data, 65);

    if (!(data[0] == 0xff && data[1] == 0x55)) {
        printf("Invalid header response\n");
    }

    uint8_t selectedMode = data[11];
    printf("Selected mode: ");
    if (selectedMode == 0xa0) printf("min");
    else if (selectedMode == 0xa1) printf("max");
    else if (selectedMode == 0xa2) printf("rotate");
    else printf("config %d", selectedMode+1);
    printf("\n");

    uint8_t length = data[9];

    uint8_t content[64];
    for (int i = 0; i<length; i++) {
        content[i] = data[10+i];
    }

    uint8_t modeCount = content[5];
    printf("Mode count: %d\n", modeCount);

    for (int i = 0; i< modeCount; i++) {
        uint32_t mode = 0;
        mode |= content[7 + i*4 + 0];
        mode |= content[7 + i*4 + 1] << 8;
        mode |= content[7 + i*4 + 2] << 16;
        mode |= content[7 + i*4 + 3] << 24;

        bool variable = (mode & 0xc0000000) == 0xc0000000;
        int voltage = ((mode & 0x7FE00) >> 9) * 2.5; // 1V == 1000
        int amperage = (mode & 0x1ff) * 1; // 1A == 1000
        if (variable) {
            int amperage = ((mode & 0x7f))*5;
            int lower = ((mode & 0xff80) >> 7) * 5;
            int upper = ((mode & 0x1ff0000) >> 16) * 5;
            // int minVoltage = (mode & 0x1ff)*5;
            // int maxVoltage = ((mode & 0xf0000000)>>28)*25;
            // printf("Mode %d:  %2d.%02dV - %2d.%02dV  0x%08x\n", i+1, minVoltage/100, minVoltage%100, maxVoltage/100, maxVoltage%100, mode);
            printf("Mode %d: %2d.%02dV - %2d.%02dV  %2d.%02dA  0x%08x\n", i+1, lower/100, lower%100, upper/100, upper%100, amperage/100, amperage%100, mode);

        }  else {
            printf("Mode %d:   %2d.%02dV %2d.%02dA   0x%08x  %s\n", i+1, voltage/100, voltage%100, amperage/100, amperage%100, mode, (variable)?"(variable)":"");
        }
    }
    

    printf("\n");
    printf("hex dump (content only): \n");
    for (int i = 0; i<length; i++) {
        printf("%02X ", content[i]);
        if ((i+1) % 16 == 0) printf("\n");
    }
    printf("\n");
}

int main(int argc, char* argv[]) {
    wchar_t buf[BUF_SIZE];

    if (hid_init() == -1) {
        fprintf(stderr, "Unable to init hidapi\n");
        return 1;
    }

    hid_device* handle = hid_open(0x0716, 0x5032, L"UPD005");

    if (handle == nullptr) {
        fprintf(stderr, "PDC device not connected\n");
        return 1;
    }

    printf("Device connected:\n");

    hid_get_manufacturer_string(handle, buf, BUF_SIZE);
    wprintf(L"Manufacturer string: %ls\n", buf);

    hid_get_product_string(handle, buf, BUF_SIZE);
    wprintf(L"Product string: %ls\n", buf);

    hid_get_serial_number_string(handle, buf, BUF_SIZE);
    wprintf(L"Serial Number: %ls\n", buf);

    doThing(handle, msg1);

    getFirmwareName(handle);

    readModes(handle);

    hid_exit();
    return 0;
}
