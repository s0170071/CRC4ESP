# CRC4ESP
Failing flash chips have been bothering me for quite some time now. Especially a faulty configuration may prevent ESPEasy from starting. So I have added MD5 hashes for ESPEasy Settings, SecuritySettings and the program code. I used MD5 because it is already part of the Arduino SDK. 

## MD5 for the config files
The size is not really an issue here as spiffs is sufficiently large. The execution time in case of the config files is negligible. 
The configuration has been extended by two hashes, the hash of the binary and the hash of the whole config struct except for the hash itself. That way it can detected if the ESPEasy version that originally wrote the  configuration has changed. As of now, there is just a note on Serial. It displays CRC OK/FAIL and binary change.

## MD5 for the program memory

checking the program memory is a bit more tricky. I suggested it as feature request here esp8266/Arduino#4165  but the reactions there were ... unfavourably cautious. So I did it myself.

Parsing the binary I found that it is comprised of five parts. 

1. The bootloader. It is not accessible for reading (returns always 0) so it cannot be checked. Anyway, we couldn't do anything about it anyway as we just would not get to check it in the first place if it was terminally faulty.
2.  irom_0. Thats where most of the program lies. It is covered by the hash. 
3. iram1_0_seg. Checked as well.
4. dram0_0_seg. This is some SPI initialized Ram. It contains the values as they are in the .bin file, but they change as you go. Inherently unstable, not usable for checksumming.
5. dram0_0_seg. Same as above so not used for checking.


The cheksum is injected into the binary by replacing an known const array "MD5_MD5_MD5_MD5_BoundariesOfTheSegmentsGoHere...". It holds 
- the md5 checksum  
- four uint32 as address of the start of sections to be checked
- four uint32 as address of the end of sections to be checked
So a total of four sections can be included in the check. Currently only 2 are used. Unused sections have start address 0

Parsing is done by the python script crc2.py (-> attached to this post). When applied to the ESPEasy.bin file (crc2.py <somefile.bin>, it outputs.

```
BINARY PART
Segments: 0x1
SEGMENT 0: memory position: 0x4010f000 to 0x4010f568 length: 0x568


BINARY PART
Segments: 0x4
SEGMENT 1: memory position: 0x40201010 to 0x40281470 length: 0x80460
SEGMENT 2: memory position: 0x40100000 to 0x40107eec length: 0x7eec
SEGMENT 3: memory position: 0x3ffe8000 to 0x3ffe8494 length: 0x494
SEGMENT 4: memory position: 0x3ffe84a0 to 0x3ffeab6c length: 0x26cc <-- CRC is here.
hash includes segments: 1 2
```


Note that the output indicates in which section of the bin file the crc dummy sting is found. If it is not found, the bin file is not modified and a warning is displayed.
Checking 500kb of irom requires 230ms @80MHz. After checking the MD5 will be available in a global array "thisBinaryMd5" and it is also displayed on the info page.
As for the dummy string: be aware that changes to the check routine may a) cause the dummy string to be moved to another section of the flash or b) get optimized away by the compiler. So always check the output of the crc2.py.
If the dummy is ever moved to a checked region of flash - that's OK, but make sure to exclude it from the calculation.


### Installation of the checksum generator

The checksum is calculated and injected into the bin file by means of a python script. It can be integrated into the toolchain as follows. 

Portable installation with Arduino IDE 1.8.5: (please feel free to provide platformio description)

1. go to your Arduino folder, then navigate to
portable/packages/esp8266/hardware/esp8266/2.3.0/ and open platform.txt
add following line at the end of the "create hex" section:

Linux:   
`recipe.objcopy.postobjcopy.1.pattern = "python" "{runtime.platform.path}/tools/crc2.py" "{build.path}/{build.project_name}.bin"`
Windows: 
`recipe.objcopy.postobjcopy.1.pattern = "c:\python27\python.exe" "{runtime.platform.path}/tools/crc2.py" "{build.path}/{build.project_name}.bin"`

 
2. put the crc2.py file into the folder 
portable/packages/esp8266/hardware/esp8266/2.3.0/tools/
