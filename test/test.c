#include <stdbool.h>
#include "reader.h"
#include "readerMock.h"
#include "unity.h"

int poz[208];
long period[208];
int oz[208];

Reader readerTest;

void init(){

  readerTest.readPacket = readPacketMock;
  readerTest.prepare = prepareMock;
  readerTest.validatePacket = validatePacket;
  readerTest.prepare();
}

void testValidationShouldOK(void){

  int count = readerTest.readPacket(poz,period,oz);

  unsigned char message[26];

  bool val = readerTest.validatePacket(message,oz,count);

  TEST_ASSERT_TRUE(val);

}

void testValidationShouldFail(void){

  int count = readerTest.readPacket(poz,period,oz);

  oz[200] = 1;  oz[201] = 1;  oz[202] = 1;  oz[203] = 1;  oz[204] = 1;
  oz[205] = 1;  oz[206] = 1;  oz[207] = 1;
  unsigned char message[26];

  bool val = readerTest.validatePacket(message,oz,count);

  TEST_ASSERT_FALSE(val);

}

int main(){
  init();
  UNITY_BEGIN();
  if(TEST_PROTECT()){
    RUN_TEST(testValidationShouldOK);
    RUN_TEST(testValidationShouldFail);
  }
  UNITY_END();
}
