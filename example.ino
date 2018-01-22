#include <md5.h>

/********************************************************************************************\
  check the program memory hash
  The const MD5_MD5_MD5_MD5_BoundariesOfTheSegmentsGoHere... needs to remain unchanged as it will be replaced by 
  - 16 bytes md5 hash, followed by
  - 4 * uint32_t start of memory segment 1-4
  - 4 * uint32_t end of memory segment 1-4
  currently there are only two segemts included in the hash. Unused segments have start adress 0.
  Execution time 520kb @80Mhz: 236ms
  Returns: 0 if hash compare fails, number of checked bytes otherwise.
  The reference hash is calculated by a .py file and injected into the binary.
  Caution: currently the hash sits in an unchecked segment. If it ever moves to a checked segment, make sure 
  it is excluded from the calculation !    
  \*********************************************************************************************/

  /*
  void dump (uint32_t addr){
       Serial.print (addr,HEX);
       Serial.print(": ");
       for (uint32_t a = addr; a<addr+16; a++)
          {  
          Serial.print  (  pgm_read_byte(a),HEX);
          Serial.print (" ");
          }
       Serial.println("");   
} 
*/

uint32_t progMemMD5check(){
    #define BufSize 10     
    const  char CRCdummy[16+32+1] = "MD5_MD5_MD5_MD5_BoundariesOfTheSegmentsGoHere...";                   // 16Bytes MD5, 32 Bytes Segment boundaries, 1Byte 0-termination. DO NOT MODIFY !
    uint32_t calcBuffer[BufSize];         
    uint32_t md5NoOfBytes = 0; 
    memcpy (calcBuffer,CRCdummy,16);                                                                      // is there still the dummy in memory ? - the dummy needs to be replaced by the real md5 after linking.
    if( memcmp (calcBuffer, "MD5_MD5_MD5_MD5_",16)==0){                                                   // do not memcmp with CRCdummy directly or it will get optimized away.
        Serial.println( F("CRC  : No program memory checksum found. Check output of crc2.py"));
        return 0;
    }
    MD5Builder md5;
    md5.begin();
    for (int l = 0; l<4; l++){                                                                            // check max segments,  if the pointer is not 0
        uint32_t *ptrStart = (uint32_t *)&CRCdummy[16+l*4]; 
        uint32_t *ptrEnd =   (uint32_t *)&CRCdummy[16+4*4+l*4]; 
        if ((*ptrStart) == 0) break;                                                                      // segment not used.
        for (uint32_t i = *ptrStart; i< (*ptrEnd) ; i=i+sizeof(calcBuffer)){                              // "<" includes last byte 
             for (int buf = 0; buf < BufSize; buf ++){
                calcBuffer[buf] = pgm_read_dword((void*)i+buf*4);                                         // read 4 bytes
                md5NoOfBytes+=sizeof(calcBuffer[0]);
             }
             md5.add((uint8_t *)&calcBuffer[0],(*ptrEnd-i)<sizeof(calcBuffer) ? (*ptrEnd-i):sizeof(calcBuffer) );     // add buffer to md5. At the end not the whole buffer. md5 ptr to data in ram.
        }
   }
   md5.calculate();
   md5.getBytes(thisBinaryMd5);
   if ( memcmp (CRCdummy, thisBinaryMd5, 16) == 0 ) {
      Serial.println(F("CRC  : program checksum       ...OK"));
      return md5NoOfBytes;
   }
   Serial.println( F("CRC  : program checksum     ...FAIL"));
   return 0; 
}

void setup() {

  Serial.begin(115200);
  Serial.print("\n\n\nboot...");
  progMemMD5check();  
 }

void loop() {
 
	for (;;) delay(1);
}
