#include <cstdint>
#include <stdio.h>
#include "TelemetryBlackBox.h"

/*
    OPIS RAMKI DANYCH:
      bit no      bit amount  description
      0-2         (3 bits)    stan rakiety (RocketState)
      3           (1 bit)     stan spadochornu
      4           (1 bit)     flaga błędu barometru
      5-7         (3 bits)    rezerwowane bity
      8-23        (16 bits)   wysokosc w stopach 0...65535
      24-31       (8 bits)    suma kontrolna XOR

    BitShifts    -> przesunięcia bitowe poszczególnych zmiennych w ramce
    BitMasks     -> maski bitowe zawierające jedynki w bitach zmiennej w ramce
    currentFrame -> aktualny stan ramki (może nie być w pełni wypełniony)
    buffer       -> historia stanów
*/

int TelemetryBlackBox::calcXORsum(uint32_t frame){
    uint8_t byte1 = (uint8_t)((0x000000FF & frame));
    uint8_t byte2 = (uint8_t)((0x0000FF00 & frame) >> 8);
    uint8_t byte3 = (uint8_t)((0x00FF0000 & frame) >> 16);
    int checkSum = byte1 ^ byte2 ^ byte3;
    return checkSum;
}

void TelemetryBlackBox::printBinary(uint32_t n){
    for (int i = 31; i >= 0; i--) {
        printf("%u", (unsigned)((n >> i) & 1u));
        if(i%8 == 0){
            printf(" ");
        }
    }
}

void TelemetryBlackBox::onRocketStateChange ( RocketState state ) {
    currentState = state;
};

void TelemetryBlackBox::onParachuteDeployed () {
    parachute = 1;
};


void TelemetryBlackBox::onBaroReading ( bool error , uint16_t value ) {
    altitude = value;
    baroError = error;
};


void TelemetryBlackBox::recordCurrentState () {
    currentFrame = 0;

    currentFrame &= BitMasks::STATE_MASK;
    currentFrame += ((currentState) << BitShifts::STATE);
    
    currentFrame &= BitMasks::PARACHUTE_MASK;
    currentFrame += ((parachute) << BitShifts::PARACHUTE);

    currentFrame &= BitMasks::BAROERROR_MASK;
    currentFrame &= BitMasks::ALTITUDE_MASK;

    currentFrame += ((altitude) << BitShifts::ALTITUDE);
    currentFrame += ((baroError) << BitShifts::BAROERROR);

    if((currentState == RocketState::DISARM) || (currentState == RocketState::LANDED)){
        currentFrame &= BitMasks::ALTITUDE_MASK;
    }

    if(baroError){
        currentFrame |= 0x00FFFF00;
    }

    currentFrame &= BitMasks::CHECKSUM_MASK;
    currentFrame += (calcXORsum(currentFrame) << BitShifts::CHECKSUM);

    lastRecordId = (lastRecordId + 1) % MAXBXBUFFER;
    buffer[lastRecordId] = currentFrame;
};

uint32_t TelemetryBlackBox::getLastDataframe (){
    uint32_t lastFrame = buffer[lastRecordId];
    printf("Ostatnia ramka:\n");
    printBinary(lastFrame);
    printf(" <- Ramka w postaci bitowej\n");
    printf(" - State: %d\n", (lastFrame & (~BitMasks::STATE_MASK)) >> BitShifts::STATE);
    printf(" - Parachute: %d\n", (lastFrame & (~BitMasks::PARACHUTE_MASK)) >> BitShifts::PARACHUTE);
    printf(" - Baro error: %d\n", (lastFrame & (~BitMasks::BAROERROR_MASK)) >> BitShifts::BAROERROR);
    printf(" - Altitude: %d\n", (lastFrame & (~BitMasks::ALTITUDE_MASK)) >> BitShifts::ALTITUDE);

    if(calcXORsum(lastFrame) != ((lastFrame & (~(BitMasks::CHECKSUM_MASK))) >> BitShifts::CHECKSUM)){
        printf("! Nie zgadza sie cotnrol sum, zwrocono 0xFFFFFFFF\n");
        return 0xFFFFFFFF;
    }
    printf("Zgadza sie control sum\n");

    return buffer[lastRecordId];
}

uint32_t TelemetryBlackBox::recordWithBadXor (){
    uint32_t lastFrame = buffer[lastRecordId];
    printf("Ostatnia ramka:\n");
    printBinary(lastFrame);
    printf(" <- Ramka w postaci bitowej\n");
    printf(" - State: %d\n", (lastFrame & (~BitMasks::STATE_MASK)) >> BitShifts::STATE);
    printf(" - Parachute: %d\n", (lastFrame & (~BitMasks::PARACHUTE_MASK)) >> BitShifts::PARACHUTE);
    printf(" - Baro error: %d\n", (lastFrame & (~BitMasks::BAROERROR_MASK)) >> BitShifts::BAROERROR);
    printf(" - Altitude: %d\n", (lastFrame & (~BitMasks::ALTITUDE_MASK)) >> BitShifts::ALTITUDE);

    lastFrame ^= (1 << 31);
    if(calcXORsum(lastFrame) != ((lastFrame & (~(BitMasks::CHECKSUM_MASK))) >> BitShifts::CHECKSUM)){
        printf("! Nie zgadza sie cotnrol sum, zwrocono 0xFFFFFFFF\n");
        return 0xFFFFFFFF;
    }
    printf("Zgadza sie control sum\n");

    return buffer[lastRecordId];
}