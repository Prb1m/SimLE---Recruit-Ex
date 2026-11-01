#pragma once
#include <cstdint>
#include <stdio.h>

#define MAXBXBUFFER 16
// maksymalny buffer

enum RocketState : uint8_t {
    DISARM = 0 ,
    ARM = 1 ,
    FLIGHT = 2 ,
    PARACHUTE = 3 ,
    LANDED = 4
};

class TelemetryBlackBox {
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

private:
    // przesuniecia bitowe czesci masek
    enum BitShifts : uint8_t {
        STATE = 0,
        PARACHUTE = 3,
        BAROERROR = 4,
        RESERVED = 5,
        ALTITUDE = 8,
        CHECKSUM = 24
    };

    // ODWROTNE maski poszczegolnych czesci ramki
    enum BitMasks : uint32_t {
        STATE_MASK     = ~(uint32_t(0b111) << BitShifts::STATE),
        PARACHUTE_MASK = ~(uint32_t(1) << BitShifts::PARACHUTE),
        BAROERROR_MASK = ~(uint32_t(1) << BitShifts::BAROERROR),
        RESERVED_MASK  = ~(uint32_t(0b111) << BitShifts::RESERVED),
        ALTITUDE_MASK  = ~(uint32_t(0xFFFF) << BitShifts::ALTITUDE),
        CHECKSUM_MASK  = ~(uint32_t(0xFF) << BitShifts::CHECKSUM)
    };

    uint32_t currentFrame = 0;      // Aktualna ramka, może nie być w pelni wypełniona
    uint32_t buffer[ MAXBXBUFFER ] = { 0 };  // Bufor kołowy do przechowywania ramek danych
    uint8_t lastRecordId = 0;       // Indeks ostatniej pozycji w buforze

    int calcXORsum(uint32_t frame){
        uint8_t byte1 = (uint8_t)((0x000000FF & frame));
        uint8_t byte2 = (uint8_t)((0x0000FF00 & frame) >> 8);
        uint8_t byte3 = (uint8_t)((0x00FF0000 & frame) >> 16);
        int checkSum = byte1 ^ byte2 ^ byte3;
        return checkSum;
    }

    void printBinary(uint32_t n){
        for (int i = 31; i >= 0; i--) {
            printf("%u", (unsigned)((n >> i) & 1u));
            if(i%8 == 0){
                printf(" ");
            }
        }
    }

public:
    TelemetryBlackBox(){};

    void onRocketStateChange ( RocketState state ) {
        currentFrame = currentFrame & (~(BitMasks::STATE_MASK));
        currentFrame += state;
        if ((state = RocketState::LANDED) || (state = RocketState::DISARM)){
            currentFrame = currentFrame & (~(BitMasks::STATE_MASK));
        }
    };
    
    void onParachuteDeployed () {
        currentFrame = currentFrame & (~(BitMasks::PARACHUTE_MASK));
        currentFrame += (1 <<  BitShifts::PARACHUTE);
    };


    /**
    * @param error - Informuje czy wystąpił błąd barometru
    * @param value - Odczyt wysokośći w stopach
    */
    void onBaroReading ( bool error , uint16_t value ) {
        if (error){
            currentFrame = currentFrame ^ BitMasks::ALTITUDE_MASK;
            currentFrame = currentFrame ^ BitMasks::BAROERROR_MASK;
        }
        else{
            currentFrame = currentFrame & (~(BitMasks::BAROERROR_MASK));
            currentFrame = currentFrame & (~(BitMasks::ALTITUDE_MASK));
            currentFrame += (value<<BitShifts::ALTITUDE);
        }
    };
    

    /**
    * @brief Pobiera bieżący stan, składa go w 32-bitową ramkę z sumą kontrolną
        i zapisuje do bufora .
    */
    void recordCurrentState () {
        lastRecordId = (lastRecordId+1)%MAXBXBUFFER;
        int checkSum = calcXORsum(currentFrame) << BitShifts::CHECKSUM;
        currentFrame += checkSum;
        buffer[lastRecordId] = currentFrame;
    };

    /**
    * @brief Funkcja pobiera ostatnią ramkę z bufora kołowego, dekoduje ją oraz
        loguje zdekodowane wartości w czytelny sposób na konsolę.
    
    * @return Ostatnia ramka zapisana do bufora, jeśli xor się zgadza.
        0xFFFFFFFF jeśli suma kontrolna jest niepoprawna.
    */
    uint32_t getLastDataframe (){
        uint32_t lastRecord = buffer[lastRecordId];
        uint32_t controlSum = (calcXORsum(currentFrame)<<24);
        
        if ((controlSum) == (lastRecord & (~(BitMasks::CHECKSUM_MASK)))){
            return lastRecord;
        }
        else{
            return 0xFFFFFFFF;
        }
    }

    void printHistory(){
        for(int i = 0; i<MAXBXBUFFER; i++){
            printf("%d :\t", i);
            printBinary(buffer[(i+lastRecordId)%MAXBXBUFFER]);
            printf("\n");
        }
    }

    void printCurrentFrame(){
        int checkSum = calcXORsum(currentFrame) << BitShifts::CHECKSUM;
        currentFrame += checkSum;
        printf("Current frame: ");
        printBinary(currentFrame);
        printf("\n");
    }

    void printAltitude(){
        uint32_t temp = currentFrame;
        temp &= ~(BitMasks::ALTITUDE_MASK);
        temp >>= BitShifts::ALTITUDE;
        printf("Altitude in binary: ");
        printBinary(temp);
        printf("    Altitude: %d\n", temp);
    }
};