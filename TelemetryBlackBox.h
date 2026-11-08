#pragma once
#include <cstdint>

enum RocketState : uint8_t {
    DISARM = 0 ,
    ARM = 1 ,
    FLIGHT = 2 ,
    PARACHUTE = 3 ,
    LANDED = 4
};

class TelemetryBlackBox {
private:
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
    static constexpr int MAXBXBUFFER = 16;
    uint32_t buffer[ MAXBXBUFFER ] = { 0 };  // Bufor kołowy do przechowywania ramek danych
    uint8_t lastRecordId = 0;       // Indeks ostatniej pozycji w buforze
    bool baroError = 0;
    uint16_t altitude = 0;
    bool parachute = 0;
    RocketState currentState = DISARM;
    int calcXORsum(uint32_t frame) ;
    void printBinary(uint32_t n) ;

public:
    void onRocketStateChange ( RocketState state ) ;

    void onParachuteDeployed () ;

    /**
    * @param error - Informuje czy wyst ąpił błąd barometru
    * @param value - Odczyt wysoko ści w stopach
    */
    void onBaroReading ( bool error , uint16_t value ) ;

    /**
    * @brief Pobiera bieżący stan , składa go w 32 - bitow ą ramk ę z sumą kontroln ą
    i zapisuje do bufora .
    */
    void recordCurrentState () ;

    /**
    * @brief Funkcja pobiera ostatni ą ramk ę z bufora kołowego , dekoduje ją oraz
    loguje zdekodowane warto ści w czytelny spos ób na konsol ę.
    * @Return Ostatnia ramka zapisana do bufora , jeśli xor się zgadza .
    0 xFFFFFFFF jeśli suma kontrolna jest niepoprawna .
    */
    uint32_t getLastDataframe () ;

    uint32_t recordWithBadXor () ;

    uint32_t recordClean () ;
};
