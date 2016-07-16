/*********************************************************
 *
 *	RaspberryPi Raspbian i2c KS103 control test code.
 *
 *  Author : Yang Liuming <dicksonliuming@gmail.com>
 *  Data   : 2016-6
 *
 *********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <string.h>


typedef unsigned int u32_t;
typedef unsigned short u16_t;
typedef unsigned char u8_t;

//#define I2C_ADDRESS    0xe8
#define REG            0x02
#define I2C_FILE_NAME   "/dev/i2c-0"
#define USAGE_MESSAGE \
        "Usage: \n" \
        " '%s -d [addr]'  " \
        " to get distance from ks103, like: ./ks103-i2c-test -d 0x74 \n" \
        " '%s -c [addr] [new addr]'  " \
        " to change ks103 directory, like: ./ks103-i2c-test -c 0x74 0x75\n" \
        " '%s -r [addr] [register]'  " \
        " to read value from [register]\n" \
        " '%s -w [addr] [register] [value]'  " \
        " to write value to [register]\n" \
        ""

static u32_t set_i2c_register(u32_t fd, u8_t addr, u8_t reg, u8_t value) {
    u8_t outbuf[2];
    struct i2c_rdwr_ioctl_data  packets;
    struct i2c_msg  messages[1];

    messages[0].addr    = addr;
    messages[0].flags   = 0;
    messages[0].len     = sizeof(outbuf);
    messages[0].buf     = outbuf;

    // The first byte indicates which register we'll write
    outbuf[0] = reg;

    // The second byte indicates the value to write
    outbuf[1] = value;

    // Transfer the i2c packets to the kernel and verify it worked
    packets.msgs = messages;
    packets.nmsgs = 1;
    if ( ioctl(fd, I2C_RDWR, &packets) < 0 ) {
        perror("Unable to send data");
        return 1;
    }

    return 0;
}

static u32_t get_i2c_register(u32_t fd, u8_t addr, u8_t reg, u8_t *val) {
    u8_t inbuf, outbuf;
    struct i2c_rdwr_ioctl_data  packets;
    struct i2c_msg  messages[2];

    // In order to read a register, we first do a "dummy write" by writing
    // 0 bytes to the register we want to read from.
    outbuf = reg;
    messages[0].addr    = addr;
    messages[0].flags   = 0;
    messages[0].len     = sizeof(outbuf);
    messages[0].buf     = &outbuf;

    messages[1].addr    = addr;
    messages[1].flags   = I2C_M_RD;     // | I2C_M_NOSTART
    messages[1].len     = sizeof(inbuf);
    messages[1].buf     = &inbuf;

    // Send the request to the kernel and get the result back
    packets.msgs    = messages;
    packets.nmsgs   = 2;
    if (ioctl(fd, I2C_RDWR, &packets) < 0 ) {
        perror("Unable to send data");
        return 1;
    }
    *val = inbuf;
    return 0;
}

static u16_t get_distance(u32_t fd, u8_t addr, u8_t reg, u16_t *distance) {
    u8_t distance_high;
    u8_t distance_low;

    set_i2c_register(fd, addr, reg, 0xb4);
    usleep(100000);     // 100ms
    if (get_i2c_register(fd, addr, reg, &distance_high))
        return 1;
    *distance = distance_high << 8;
    if (get_i2c_register(fd, addr, reg+1, &distance_low))
        return 1;
    *distance |= distance_low;

    return 0;
}

static u8_t change_directory(u32_t fd, u8_t addr, u8_t new_addr) {
    if (set_i2c_register(fd, addr, REG, 0x9a)) {
        return 1;
    }
    usleep(1000);  // 1ms
    if (set_i2c_register(fd, addr, REG, 0x92)) {
        return 1;
    }
    usleep(1000);  // 1ms
    if (set_i2c_register(fd, addr, REG, 0x9e)) {
        return 1;
    }
    usleep(1000);  // 1ms
    if (set_i2c_register(fd, addr, REG, new_addr)) {
        return 1;
    }
    usleep(100000); // 100ms

    return 0;

}

int main(int argc, char const *argv[]) {
    int fd;

     fd = open("/dev/i2c-1", O_RDWR);
     if (fd < 0) {
         printf("i2c open error\n");
         exit(1);
     }
     printf("open i2c port success, fd = %d\n", fd);

     // to get the distance from the i2c device which user giving address 
     if(3 == argc && !strcmp(argv[1], "-d")) {
         u32_t addr = strtol(argv[2], NULL, 0);
         u16_t distance = 0;
         while(1) {
             if ( get_distance(fd, addr, REG, &distance)) {
                 perror("Unable to get distance");
                 exit(1);
             }
             else {
                 printf("distance is %d mm\n", distance);
             }
             //usleep();    // 0.5s
             sleep(0.5);
        }
     }

     // to get the distance from the i2c device which user giving address, from addr1 to check
     // argv[3] times to each i2c device.
     // for example, if you input: "./ks103 -m 0x72 5" , not contain quot, that is to say, check 
     // 0x72, 0x73, 0x74, 0x75, 0x76 i2c devices.
     else if(4 == argc && !strcmp(argv[1], "-m")) {
         u8_t begin_addr = strtol(argv[2], NULL, 0);
         u32_t i2c_devices_number = strtol(argv[3], NULL, 0);
         u8_t addr_count = 0;
         u16_t distance = 0;
         while(1) {
             for(addr_count = 0; addr_count < i2c_devices_number; addr_count++) {
                 if (get_distance(fd, begin_addr+addr_count, REG, &distance)) {
                     printf("Unable to get distance, %x", begin_addr+addr_count);
                     exit(1);
                 }
                 else {
                     printf("num.%d distance is %d mm\n", addr_count+1, distance);
                 }
             }
             sleep(1);
             printf("\n");
         }
     }

     // to change the address to the i2c device which user giving address, the last 
     // argument: argv[2], is the address you want to change to.
     else if(4 == argc && !strcmp(argv[1], "-c")) {
         u8_t addr = strtol(argv[2], NULL, 0);
         u8_t new_addr = strtol(argv[3], NULL, 0);
         if (change_directory(fd, addr, new_addr)) {
             printf("Unable to change directory\n");
         }
         else {
             printf("old directory is %x, new directory is %x\n", addr, new_addr);
         }
     }

     // to read a register value from the i2c device which user giving address, in attention 
     // that this mode don't send any words to i2c device
     else if(4 == argc && !strcmp(argv[1], "-r")) {
         u32_t addr = strtol(argv[2], NULL, 0);
         u32_t reg = strtol(argv[3], NULL ,0);
         u8_t value;
         if ( get_i2c_register(fd, addr, reg, &value) ) {
                printf("Unable to get register!\n");
         }
         else {
             printf("Register %d: %d (%x)\n", reg, (u32_t)value, (u32_t)value);
         }
     }

     // to set a register value to the i2c device which user giving address, argv[2] is address,
     // argv[3] is register number, argv[4] is value you want to write.
     else if( 5 == argc && !strcmp(argv[1], "-w")) {
         u32_t addr = strtol(argv[2], NULL, 0);
         u32_t reg = strtol(argv[3], NULL, 0);
         u32_t value = strtol(argv[4], NULL, 0);
         if (set_i2c_register(fd, addr, reg, value)) {
             printf("Set register %x: %d (%x)\n", reg, value, value);
         }
     }
     else {
         fprintf(stderr, USAGE_MESSAGE, argv[0], argv[0], argv[0]);
     }

     close(fd);
     return 0;
}
