#include <stdio.h>
#include <cstdint>
#include "TelemetryBlackBox.h"

using namespace std;

int calcXORsum(uint32_t frame){
    uint8_t byte1 = (uint8_t)((0x000000FF & frame));
    uint8_t byte2 = (uint8_t)((0x0000FF00 & frame) >> 8);
    uint8_t byte3 = (uint8_t)((0x00FF0000 & frame) >> 16);
    int checkSum = byte1 ^ byte2 ^ byte3;
    return checkSum;
}

void regularTest(TelemetryBlackBox box, int i){
    box.onRocketStateChange(RocketState::FLIGHT);
    box.onParachuteDeployed();
    box.onBaroReading(0, 1562);
    box.recordCurrentState();
    printf("Funkcjonalnosc podstawowa ---\n");
    uint32_t output = box.getLastDataframe();
    uint32_t correct = (1562 << 8) + 0b00001010;
    uint32_t xorSum = calcXORsum(correct);
    correct += (xorSum << 24);
    if (output == correct){
        printf("Test numer %d: PASS\n", i);
    }
    else{
        printf("Test numer %d: FAIL\n", i);
    }
}

void zeroAltTestLanded(TelemetryBlackBox box, int i){
    box.onRocketStateChange(RocketState::LANDED);
    box.onBaroReading(0, 200);
    box.recordCurrentState();
    printf("Zerowanie altitude przy landed---\n");
    uint32_t output = box.getLastDataframe();
    uint32_t correct = 0b0100;
    uint32_t xorSum = calcXORsum(correct);
    correct += (xorSum << 24);
    if (output == correct){
        printf("Test numer %d: PASS\n", i);
    }
    else{
        printf("Test numer %d: FAIL\n", i);
    }
}

void zeroAltTestDisarm(TelemetryBlackBox box, int i){
    box.onRocketStateChange(RocketState::DISARM);
    box.onBaroReading(0, 200);
    box.recordCurrentState();
    printf("Zerowanie altitude przy disarm ---\n");
    uint32_t output = box.getLastDataframe();
    uint32_t correct = 0b0000;
    uint32_t xorSum = calcXORsum(correct);
    correct += (xorSum << 24);
    if (output == correct){
        printf("Test numer %d: PASS\n", i);
    }
    else{
        printf("Test numer %d: FAIL\n", i);
    }
}

void baroErrorTest(TelemetryBlackBox box, int i){
    box.onRocketStateChange(RocketState::FLIGHT);
    box.onBaroReading(1, 1000);
    box.recordCurrentState();
    printf("Blad odczytu ---\n");
    uint32_t output = box.getLastDataframe();
    uint32_t correct = 0b00010010 + 0x00FFFF00;
    uint32_t xorSum = calcXORsum(correct);
    correct += (xorSum << 24);
    if (output == correct){
        printf("Test numer %d: PASS\n", i);
    }
    else{
        printf("Test numer %d: FAIL\n", i);
    }
}

void historyTest(TelemetryBlackBox box, int i){
    box.onRocketStateChange(RocketState::FLIGHT);
    box.onBaroReading(0, 1500);
    box.recordCurrentState();
    box.getLastDataframe();
    box.onBaroReading(0, 1750);
    box.recordCurrentState();
    printf("\nTest historii ---\n");
    uint32_t output = box.getLastDataframe();
    uint32_t correct = (1750 << 8) + 0b00000010;
    uint32_t xorSum = calcXORsum(correct);
    correct += (xorSum << 24);
    if (output == correct){
        printf("Test numer %d: PASS\n", i);
    }
    else{
        printf("Test numer %d: FAIL\n", i);
    }
}

void checkSumTest(TelemetryBlackBox box, int i){
    box.onRocketStateChange(RocketState::FLIGHT);
    box.onParachuteDeployed();
    box.onBaroReading(0, 1562);
    box.recordCurrentState();
    printf("Funkcjonalnosc checksum ---\n");
    uint32_t output = box.recordWithBadXor();
    if (output == 0xFFFFFFFF){
        printf("Test numer %d: PASS\n", i);
    }
    else{
        printf("Test numer %d: FAIL\n", i);
    }
}

void checkCircleBuffer(TelemetryBlackBox box, int i){
    for(int j = 0; j < 20; j++){
        box.onRocketStateChange(RocketState::FLIGHT);
        box.onBaroReading(0, j*100+j);
        box.recordCurrentState();
    }
    box.onRocketStateChange(RocketState::FLIGHT);
    box.onBaroReading(0, 3000);
    box.recordCurrentState();
    printf("Funkcjonalnosc bufora kolowego ---\n");
    uint32_t output = box.getLastDataframe();
    uint32_t correct = (3000 << 8) + 0b00000010;
    uint32_t xorSum = calcXORsum(correct);
    correct += (xorSum << 24);
    if (output == correct){
        printf("Test numer %d: PASS\n", i);
    }
    else{
        printf("Test numer %d: FAIL\n", i);
    }
}

int main(){
    // lista testow
    void (*tests[])(TelemetryBlackBox object, int i) = {
        regularTest,
        zeroAltTestLanded,
        zeroAltTestDisarm,
        baroErrorTest,
        historyTest,
        checkSumTest,
        checkCircleBuffer
    };
 
    TelemetryBlackBox testBox;

    // wykonywanie testow
    int counter = 1;
    for (auto test : tests){
        printf("\n");
        test(testBox, counter++);
    }
    printf("\n");

    return 0;
}