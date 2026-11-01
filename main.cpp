#include <stdio.h>
#include <cstdint>
#include "TelemetryBlackBox.cpp"

using namespace std;

void testAltitudeReading(TelemetryBlackBox box){
    printf("Test 1 - testowanie funkcjonalnosci mierzenia wysokosci:\n");
    box.onRocketStateChange(static_cast<RocketState>(2));
    box.onBaroReading(0, 455);
    box.printAltitude();
}

void testBarError(TelemetryBlackBox box){
    printf("Test 2 - testowanie funkcjonalności bar error:\n");
    box.onRocketStateChange(static_cast<RocketState>(2));
    box.onBaroReading(1, 432);
    box.printCurrentFrame();
}

void testXORError(TelemetryBlackBox box){
    printf("Test 3 - testowanie funkcjonalności XOR:\n");
    printf("test wykonany dla baroerror=1, zeby byl wypelniony jedynkami\n");
    box.onRocketStateChange(static_cast<RocketState>(2));
    box.onBaroReading(1, 0);
    box.printCurrentFrame();
}

void testHistory(TelemetryBlackBox box){
    
    printf("Test 4 - testowanie histori\n");

    box.onRocketStateChange(static_cast<RocketState>(1));
    box.recordCurrentState();
    box.onRocketStateChange(static_cast<RocketState>(2));
    box.recordCurrentState();
    for(int i = 0; i < 8; i++){
        box.onBaroReading(0, i);
        box.recordCurrentState();
    }
    box.onRocketStateChange(static_cast<RocketState>(3));
    box.recordCurrentState();
    box.onRocketStateChange(static_cast<RocketState>(4));
    box.recordCurrentState();


    box.printHistory();
}

void testParachute(TelemetryBlackBox box){
    printf("Test 5 - testowanie funkcjonalności spadochronu\n");
    box.onParachuteDeployed();
    box.printCurrentFrame();
}

void testGetFrame(TelemetryBlackBox box){
    printf("Test 6 - pobieranie ramki\n");
    box.onRocketStateChange(static_cast<RocketState>(2));
    box.onBaroReading(1, 0);
    box.recordCurrentState();
    printf("ramka pobrana: %d\n", box.getLastDataframe());
}

int main(){
    // lista testow
    void (*tests[])(TelemetryBlackBox object) = {
        testAltitudeReading,
        testBarError,
        testXORError,
        testHistory,
        testParachute,
        testGetFrame
    };
 
    TelemetryBlackBox testBox;

    // wykonywanie testow
    for (auto test : tests){
        printf("\n");
        test(testBox);
    }

    return 0;
}