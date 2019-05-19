#include <hidapi.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <array>
#include <vector>

const int BUF_SIZE = 255;

// Init/ get info      vv  vv - always ff 55   vv - always 08
uint8_t msg1[65] =  "\xff\x55\x21\xfc\xdc\x4f\x08\xa2\x03\x00\x00\x00\x00\x00\x00\x00" \
                    "\xa4\x00\xa6\x00\x06\xcd\x00\x10\xc0\xf3\x1b\x00\x4d\x13\x00\x10" \
                    "\x06\xcd\x00\x10\x00\x00\x00\xc0\x00\x00\x00\x00\x00\x00\x00\x00" \
                    "\x03\x00\x00\x00\x00\x00\x00\x40\x00\x00\x00\x00\xf8\xcc\x18\x5e";

// Get device name
uint8_t msg2[65] =  "\xff\x55\x21\xfc\xdc\x4f\x08\xa2\x0a\x05\x00\x38\x00\x08\x0f\x00" \
                    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
                    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
                    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x5e\xa4";

// Get info?
uint8_t msg3[65] =  "\xff\x55\x21\xfc\xdc\x4f\x08\xa2\x0a\x05\x00\xfc\x00\x08\x34\x00" \
                    "\xc0\xf3\x1b\x00\x38\xf4\x1b\x00\x00\xf9\x5e\x02\xa3\x19\xd2\x00" \
                    "\x98\x35\xd6\x00\xa4\xf8\x5e\x02\x06\x3e\xd2\x00\xf8\xf8\x5e\x02" \
                    "\x98\x35\xd6\x00\x50\x00\x00\x00\x44\x00\x00\x00\x43\x00\xc2\x08";

void sendCommand(hid_device* handle, uint8_t cmd[64]) {
    uint8_t data[65];
    data[0] = 0;

    memcpy(data + 1, cmd, 64);
    hid_write(handle, data, 65);
}

std::vector<uint8_t> readResponse(hid_device* handle) {
    std::array<uint8_t, 64> data;
    hid_read(handle, data.data(), 64);
    
    if (!(data[0] == 0xff && data[1] == 0x55)) {
        printf("Invalid header response\n");
    }

    char checksum[3] = {0};
    for (int ptr = 0; ptr < 0x36; ptr += 3) {
        checksum[0] += data[ptr+8];
        checksum[1] += data[ptr+9];
        checksum[2] += data[ptr+10];
    } 

    if ((uint8_t)(checksum[0] + checksum[1] + checksum[2]) != data[62]) {
        printf("Invalid checksum\n");
    }
    
    uint8_t length = data[9];

    std::vector<uint8_t> content;
    content.resize(length);

    for (int i = 0; i<length; i++) {
        content[i] = data[10+i];
    }
    
    return content;
}

void doThing(hid_device* handle, uint8_t buffer[65]) {
    sendCommand(handle, buffer);
    auto data = readResponse(handle);
}

void getFirmwareName(hid_device* handle) {
    sendCommand(handle, msg2);

    auto data = readResponse(handle);

    printf("Firmware: ");
    for (auto c : data) printf("%c", c);
    printf("\n");
}

void readModes(hid_device* handle) {
    sendCommand(handle, msg3);

    auto data = readResponse(handle);

    int chargerType = data[35];
    printf("Charger type: PD%d\n", ((chargerType&0xc0)>>6) + 1);

    uint8_t selectedMode = data[1];
    printf("Selected mode: ");
    if (selectedMode == 0xa0) printf("min");
    else if (selectedMode == 0xa1) printf("max");
    else if (selectedMode == 0xa2) printf("rotate");
    else printf("%d", selectedMode+1);

    int selectedVoltage = (data[2] | data[3]<<8) / 10;
    printf("    (savedVoltage: %2d.%02dV)\n", selectedVoltage/100, selectedVoltage%100);


    uint8_t modeCount = data[5];

    for (int i = 0; i< modeCount; i++) {
        uint32_t mode = 0;
        mode |= data[7 + i*4 + 0];
        mode |= data[7 + i*4 + 1] << 8;
        mode |= data[7 + i*4 + 2] << 16;
        mode |= data[7 + i*4 + 3] << 24;

        bool variable = (mode & 0xc0000000) == 0xc0000000;
        if (variable) {
            int amperage = ((mode & 0x7f))*5;
            int minVoltage = ((mode & 0xff80) >> 7) * 5;
            int maxVoltage = ((mode & 0x1ff0000) >> 16) * 5;
            printf("Mode %d:   %2d.%02dV - %2d.%02dV %2d.%02dA\n", i+1, minVoltage/100, minVoltage%100, maxVoltage/100, maxVoltage%100, amperage/100, amperage%100);
        }  else {
            int voltage = ((mode & 0x7FE00) >> 9) * 2.5; 
            int amperage = mode & 0x1ff; 
            printf("Mode %d:            %2d.%02dV %2d.%02dA\n", i+1, voltage/100, voltage%100, amperage/100, amperage%100);
        }
    }
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

    doThing(handle, msg1);

    getFirmwareName(handle);

    readModes(handle);

    hid_exit();
    return 0;
}
