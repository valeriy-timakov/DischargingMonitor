//
// Created by valti on 03.08.2022.
//



#include "platform.h"
//#include "OneWireHub_config.h" // outsource configfile


constexpr uint32_t operator "" _us(const unsigned long long int time_us) // user defined literal used in config
{
    return uint32_t(time_us * microsecondsToClockCycles(1) / VALUE_IPL); // note: microsecondsToClockCycles == speed in MHz....
    // TODO: overflow detection would be nice, but literals are allowed with return-only, not solvable ATM
}

// Reset: every low-state of the master between MIN & MAX microseconds will be recognized as a Reset
constexpr uint32_t ONEWIRE_TIME_RESET_TIMEOUT        = {  5000_us };        // for not hanging to long in resetADS-detection, lower value is better for more responsive applications, but can miss resets
constexpr uint32_t ONEWIRE_TIME_RESET_MIN[2]         = {   430_us, 48_us }; // should be 480
constexpr uint32_t ONEWIRE_TIME_RESET_MAX[2]         = {   960_us, 80_us }; // from ds2413

// Presence: slave waits TIMEOUT and emits a low state after the resetADS with ~MIN length, if the bus stays low after that and exceeds MAX the hub will issue an error
constexpr uint32_t ONEWIRE_TIME_PRESENCE_TIMEOUT     = {    20_us };        // probe measures 25us, duration of high state between resetADS and presence
constexpr uint32_t ONEWIRE_TIME_PRESENCE_MIN[2]      = {   160_us,  8_us }; // was 125
constexpr uint32_t ONEWIRE_TIME_PRESENCE_MAX[2]      = {   480_us, 32_us }; // should be 280, was 480


// read and write from the viewpoint of the slave!!!!
constexpr uint32_t ONEWIRE_TIME_READ_MIN[2]          = {    20_us,  4_us }; // should be 15, was 30, says when it is safe to read a valid bit
constexpr uint32_t ONEWIRE_TIME_READ_MAX[2]          = {    60_us, 10_us }; // low states (zeros) of a master should not exceed this time in a slot
constexpr uint32_t ONEWIRE_TIME_WRITE_ZERO[2]        = {    30_us,  8_us }; // the hub holds a zero for this long


constexpr uint32_t ONEWIRE_TIME_MSG_HIGH_TIMEOUT     = { 15000_us };        // there can be these inactive / high timeperiods after resetADS / presence, this value defines the timeout for these
constexpr uint32_t ONEWIRE_TIME_SLOT_MAX[2]          = {   135_us, 30_us }; // should be 120, measured from falling edge to next falling edge


constexpr uint32_t timeUsToLoops(const uint16_t time_us)
{
    return (time_us * microsecondsToClockCycles(1) / VALUE_IPL);
}


using mask_t = uint8_t;

constexpr uint32_t VALUE1k      { 1000 }; // commonly used constant
constexpr uint32_t TIMEOW_MAX   { 4294967295 };   // arduino does not support std-lib...

enum class Error : uint8_t {
    NO_ERROR                   = 0,
    READ_TIMESLOT_TIMEOUT      = 1,
    WRITE_TIMESLOT_TIMEOUT     = 2,
    WAIT_RESET_TIMEOUT         = 3,
    VERY_LONG_RESET            = 4,
    VERY_SHORT_RESET           = 5,
    PRESENCE_LOW_ON_LINE       = 6,
    READ_TIMESLOT_TIMEOUT_LOW  = 7,
    AWAIT_TIMESLOT_TIMEOUT_HIGH = 8,
    PRESENCE_HIGH_ON_LINE      = 9,
    INCORRECT_ONEWIRE_CMD      = 10,
    INCORRECT_SLAVE_USAGE      = 11,
    TRIED_INCORRECT_WRITE      = 12,
    FIRST_TIMESLOT_TIMEOUT     = 13,
    FIRST_BIT_OF_BYTE_TIMEOUT  = 14,
    RESET_IN_PROGRESS          = 15
};


bool sendBit(bool value);                                                 // returns 1 if error occured
bool send(uint8_t dataByte);                                              // returns 1 if error occured
bool send(const uint8_t address[], uint8_t data_length = 1);              // returns 1 if error occured
bool send(const uint8_t address[], uint8_t data_length, uint16_t &crc16); // returns 1 if error occured
bool    recvBit(void);
bool    recv(uint8_t address[], uint8_t data_length = 1);                 // returns 1 if error occured
bool    recv(uint8_t address[], uint8_t data_length, uint16_t &crc16);    // returns 1 if error occured
void  raiseSlaveError(uint8_t cmd = 0);
void    duty();
static constexpr bool    od_mode { false };

Error   _error;

io_reg_t          pin_bitMask;
volatile io_reg_t *pin_baseReg;



bool checkReset(void);      // returns true if error occured
bool showPresence(void);    // returns true if error occured
bool recvAndProcessCmd();   // returns true if error occured

void wait(uint32_t loops_wait);
void wait(uint16_t timeout_us);
void sendID();

inline __attribute__((always_inline))
uint32_t waitLoopsWhilePinIs(volatile uint32_t retries, bool pin_value = false);




static constexpr uint8_t    PAGE_COUNT          { 4 };
static constexpr uint8_t    PAGE_SIZE           { 32 }; // bytes
static constexpr uint8_t    PAGE_MASK           { PAGE_SIZE - 1 };

static constexpr uint8_t    MEM_SIZE            { PAGE_COUNT * PAGE_SIZE }; // bytes

static constexpr uint8_t    STATUS_SIZE         { 8 };

static constexpr uint8_t    STATUS_WP_PAGES     { 0x00 }; // 1 byte -> Page write protection and page used status
static constexpr uint8_t    STATUS_PG_REDIR     { 0x01 }; // 4 byte -> Page redirection
static constexpr uint8_t    STATUS_UNDEF_B1     { 0x05 }; // 2 byte -> reserved / undefined
static constexpr uint8_t    STATUS_FACTORYP     { 0x07 }; // 2 byte -> factoryprogrammed 0x00



uint8_t  memory[MEM_SIZE];    // 4 pages of 32 bytes
uint8_t  status[STATUS_SIZE]; // eprom status bytes:
uint8_t  sizeof_memory;       // device specific "real" size

uint8_t  translateRedirection(uint8_t source_address);

static constexpr uint8_t family_code = 0x09; // the ds2502
void    clearMemory(void);
void clearStatus(void);
uint8_t writeStatus(uint8_t address, uint8_t value);
void    setPageProtection(uint8_t page);
bool    getPageProtection(uint8_t page);

void    setPageUsed(uint8_t page);
bool    getPageUsed(uint8_t page);

bool    setPageRedirection(uint8_t page_source, uint8_t page_destin);
uint8_t getPageRedirection(uint8_t page);


uint8_t ID[8];
static uint8_t crc8(const uint8_t data[], uint8_t data_size, uint8_t crc_init = 0);
static uint16_t crc16(const uint8_t address[], uint8_t len, uint16_t init = 0);
static uint16_t crc16(uint8_t value, uint16_t crc);




//DS2502( 0x28, 0x0D, 0x01, 0x08, 0x0B, 0x02, 0x0A);
void initDS5202(const uint8_t pin, uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7)
{
    _error = Error::NO_ERROR;
    pin_bitMask = PIN_TO_BITMASK(pin);
    pin_baseReg = PIN_TO_BASEREG(pin);
    pinMode(pin, INPUT); // first port-access should by done by this FN, does more than DIRECT_MODE_....
    DIRECT_WRITE_LOW(pin_baseReg, pin_bitMask);
    static_assert(VALUE_IPL, "Your architecture has not been calibrated yet, please run examples/debug/calibrate_by_bus_timing and report instructions per loop (IPL) to https://github.com/orgua/OneWireHub");


    ID[0] = ID1;
    ID[1] = ID2;
    ID[2] = ID3;
    ID[3] = ID4;
    ID[4] = ID5;
    ID[5] = ID6;
    ID[6] = ID7;
    ID[7] = crc8(ID, 7);

    static_assert(MEM_SIZE < 256, "Implementation does not cover the whole address-space");

    clearMemory();
    clearStatus();

    if ((ID1 == 0x11) || (ID1 == 0x91))
    {
        // when set to DS2501, the upper two memory pages are not accessible, always read 0xFF
        for (uint8_t page = 2; page < PAGE_COUNT; ++page)
        {
            setPageUsed(page);
            setPageProtection(page);
            sizeof_memory = 64;
        }
    }
    else
    {
        sizeof_memory = 128;    // must be DS2502 then
    }
}


bool pollDS5202()
{
    _error = Error::NO_ERROR;
    while (true)
    {
        if (checkReset())           return false;
        // Reset is complete, tell the master we are present
        if (showPresence())         return false;
        //Now that the master should know we are here, we will get a command from the master
        if (recvAndProcessCmd())    return false;
        // on total success we want to start again, because the next resetADS could only be ~125 us away
    }
}

bool checkReset(void) // there is a specific high-time needed before a resetADS may occur -->  >120us
{

    DIRECT_MODE_INPUT(pin_baseReg, pin_bitMask);

    // is entered if there are two resets within a given time (timeslot-detection can issue this skip)
    if (_error == Error::RESET_IN_PROGRESS)
    {
        _error = Error::NO_ERROR;
        if (waitLoopsWhilePinIs(ONEWIRE_TIME_RESET_MIN[od_mode] - ONEWIRE_TIME_SLOT_MAX[od_mode] - ONEWIRE_TIME_READ_MAX[od_mode], false) == 0) // last number should read: max(ONEWIRE_TIME_WRITE_ZERO,ONEWIRE_TIME_READ_MAX)
        {

            waitLoopsWhilePinIs(ONEWIRE_TIME_RESET_MAX[0], false); // showPresence() wants to start at high, so wait for it
            return false;
        }
    }

    if (!DIRECT_READ(pin_baseReg, pin_bitMask)) return true; // just leave if pin is Low, don't bother to wait, TODO: really needed?

    // wait for the bus to become low (master-controlled), since we are polling we don't know for how long it was zero
    if (waitLoopsWhilePinIs(ONEWIRE_TIME_RESET_TIMEOUT, true) == 0)
    {
        //_error = Error::WAIT_RESET_TIMEOUT;
        return true;
    }

    const uint32_t loops_remaining = waitLoopsWhilePinIs(ONEWIRE_TIME_RESET_MAX[0], false);

    // wait for bus-release by master
    if (loops_remaining == 0)
    {
        _error = Error::VERY_LONG_RESET;
        return true;
    }


    // If the master pulled low for to short this will trigger an error
    //if (loops_remaining > (ONEWIRE_TIME_RESET_MAX[0] - ONEWIRE_TIME_RESET_MIN[od_mode])) _error = Error::VERY_SHORT_RESET; // could be activated again, like the error above, errorhandling is mature enough now

    return (loops_remaining > (ONEWIRE_TIME_RESET_MAX[0] - ONEWIRE_TIME_RESET_MIN[od_mode]));
}
bool showPresence(void)
{
    static_assert(ONEWIRE_TIME_PRESENCE_MAX[0] > ONEWIRE_TIME_PRESENCE_MIN[0], "Timings are wrong");

    // Master will delay it's "Presence" check (bus-read)  after the resetADS
    waitLoopsWhilePinIs(ONEWIRE_TIME_PRESENCE_TIMEOUT, true); // no pinCheck demanded, but this additional check can cut waitTime

    // pull the bus low and hold it some time
    DIRECT_WRITE_LOW(pin_baseReg, pin_bitMask);
    DIRECT_MODE_OUTPUT(pin_baseReg, pin_bitMask);    // drive output low

    wait(ONEWIRE_TIME_PRESENCE_MIN[od_mode]); // stays till the end, because it drives the bus low itself

    DIRECT_MODE_INPUT(pin_baseReg, pin_bitMask);     // allow it to float

    // When the master or other slaves release the bus within a given time everything is fine
    if (waitLoopsWhilePinIs((ONEWIRE_TIME_PRESENCE_MAX[od_mode] - ONEWIRE_TIME_PRESENCE_MIN[od_mode]), false) == 0)
    {
        _error = Error::PRESENCE_LOW_ON_LINE;
        return true;
    }

    return false;
}
bool recvAndProcessCmd(void)
{
    uint8_t address[8], cmd;

    recv(&cmd);

    if (_error == Error::RESET_IN_PROGRESS) return false; // stay in poll()-loop and trigger another datastream-detection
    if (_error != Error::NO_ERROR)          return true;

    switch (cmd)
    {
        case 0x69: // overdrive MATCH ROM
        case 0x55: // MATCH ROM - Choose/Select ROM
            if (recv(address, 8)) break;
            duty();
            break;

        case 0x3C: // overdrive SKIP ROM
        case 0xCC: // SKIP ROM
            duty();
            break;

        case 0x0F: // OLD READ ROM
        case 0x33: // READ ROM
            sendID();
            return false;


        case 0xA5: // RESUME COMMAND
            duty();
            break;

        default: // Unknown command
            _error = Error::INCORRECT_ONEWIRE_CMD;
    }

    if (_error == Error::RESET_IN_PROGRESS) return false;

    return (_error != Error::NO_ERROR);
}














void sendID() {
    send(ID, 8);
}




// info: check for errors after calling and break/return if possible, returns true if error is detected
// NOTE: if called separately you need to handle interrupts, should be disabled during this FN
bool sendBit(const bool value)
{
    const bool writeZero = !value;

    // Wait for bus to rise HIGH, signaling end of last timeslot
    uint32_t retries = ONEWIRE_TIME_SLOT_MAX[od_mode];
    while ((DIRECT_READ(pin_baseReg, pin_bitMask) == 0) && (--retries != 0));
    if (retries == 0)
    {
        _error = Error::RESET_IN_PROGRESS;
        return true;
    }

    // Wait for bus to fall LOW, start of new timeslot
    retries = ONEWIRE_TIME_MSG_HIGH_TIMEOUT;
    while ((DIRECT_READ(pin_baseReg, pin_bitMask) != 0) && (--retries != 0));
    if (retries == 0)
    {
        _error = Error::AWAIT_TIMESLOT_TIMEOUT_HIGH;
        return true;
    }

    // first difference to inner-loop of read()
    if (writeZero)
    {
        DIRECT_MODE_OUTPUT(pin_baseReg, pin_bitMask);
        retries = ONEWIRE_TIME_WRITE_ZERO[od_mode];
    }
    else
    {
        retries = ONEWIRE_TIME_READ_MAX[od_mode];
    }

    while ((DIRECT_READ(pin_baseReg, pin_bitMask) == 0) && (--retries != 0)); // TODO: we should check for (!retries) because there could be a resetADS in progress...
    DIRECT_MODE_INPUT(pin_baseReg, pin_bitMask);

    return false;
}


// should be the prefered function for writes, returns true if error occured
bool send(const uint8_t address[], const uint8_t data_length)
{
    noInterrupts(); // will be enabled at the end of function
    DIRECT_WRITE_LOW(pin_baseReg, pin_bitMask);
    DIRECT_MODE_INPUT(pin_baseReg, pin_bitMask);
    uint8_t bytes_sent = 0;

    for ( ; bytes_sent < data_length; ++bytes_sent)             // loop for sending bytes
    {
        const uint8_t dataByte = address[bytes_sent];

        for (uint8_t bitMask = 0x01; bitMask != 0; bitMask <<= 1)    // loop for sending bits
        {
            if (sendBit(static_cast<bool>(bitMask & dataByte)))
            {
                if ((bitMask == 0x01) && (_error == Error::AWAIT_TIMESLOT_TIMEOUT_HIGH)) _error = Error::FIRST_BIT_OF_BYTE_TIMEOUT;
                interrupts();
                return true;
            }
        }
    }
    interrupts();
    return (bytes_sent != data_length);
}

bool send(const uint8_t address[], const uint8_t data_length, uint16_t &crc16)
{
    noInterrupts(); // will be enabled at the end of function
    DIRECT_WRITE_LOW(pin_baseReg, pin_bitMask);
    DIRECT_MODE_INPUT(pin_baseReg, pin_bitMask);
    uint8_t bytes_sent = 0;

    for ( ; bytes_sent < data_length; ++bytes_sent)             // loop for sending bytes
    {
        uint8_t dataByte = address[bytes_sent];

        for (uint8_t counter = 0; counter < 8; ++counter)       // loop for sending bits
        {
            if (sendBit(static_cast<bool>(0x01 & dataByte)))
            {
                if ((counter == 0) && (_error ==Error::AWAIT_TIMESLOT_TIMEOUT_HIGH)) _error = Error::FIRST_BIT_OF_BYTE_TIMEOUT;
                interrupts();
                return true;
            }

            const uint8_t mix = ((uint8_t) crc16 ^ dataByte) & static_cast<uint8_t>(0x01);
            crc16 >>= 1;
            if (mix != 0)  crc16 ^= static_cast<uint16_t>(0xA001);
            dataByte >>= 1;
        }
    }
    interrupts();
    return (bytes_sent != data_length);
}

bool send(const uint8_t dataByte)
{
    return send(&dataByte,1);
}

// NOTE: if called separately you need to handle interrupts, should be disabled during this FN
bool recvBit(void)
{
    // Wait for bus to rise HIGH, signaling end of last timeslot
    uint32_t retries = ONEWIRE_TIME_SLOT_MAX[od_mode];
    while ((DIRECT_READ(pin_baseReg, pin_bitMask) == 0) && (--retries != 0));
    if (retries == 0)
    {
        _error = Error::RESET_IN_PROGRESS;
        return true;
    }

    // Wait for bus to fall LOW, start of new timeslot
    retries = ONEWIRE_TIME_MSG_HIGH_TIMEOUT;
    while ((DIRECT_READ(pin_baseReg, pin_bitMask) != 0) && (--retries != 0));
    if (retries == 0)
    {
        _error = Error::AWAIT_TIMESLOT_TIMEOUT_HIGH;
        return true;
    }

    // wait a specific time to do a read (data is valid by then), // first difference to inner-loop of write()
    retries = ONEWIRE_TIME_READ_MIN[od_mode];
    while ((DIRECT_READ(pin_baseReg, pin_bitMask) == 0) && (--retries != 0));

    return (retries > 0);
}


bool recv(uint8_t address[], const uint8_t data_length)
{
    noInterrupts(); // will be enabled at the end of function
    DIRECT_WRITE_LOW(pin_baseReg, pin_bitMask);
    DIRECT_MODE_INPUT(pin_baseReg, pin_bitMask);

    uint8_t bytes_received = 0;
    for ( ; bytes_received < data_length; ++bytes_received)
    {
        uint8_t value = 0;

        for (uint8_t bitMask = 0x01; bitMask != 0; bitMask <<= 1)
        {
            if (recvBit())                 value |= bitMask;
            if (_error != Error::NO_ERROR)
            {
                if ((bitMask == 0x01) && (_error ==Error::AWAIT_TIMESLOT_TIMEOUT_HIGH)) _error = Error::FIRST_BIT_OF_BYTE_TIMEOUT;
                interrupts();
                return true;
            }
        }

        address[bytes_received] = value;

    }

    interrupts();
    return (bytes_received != data_length);
}


// should be the prefered function for reads, returns true if error occured
bool recv(uint8_t address[], const uint8_t data_length, uint16_t &crc16)
{
    noInterrupts(); // will be enabled at the end of function
    DIRECT_WRITE_LOW(pin_baseReg, pin_bitMask);
    DIRECT_MODE_INPUT(pin_baseReg, pin_bitMask);

    uint8_t bytes_received = 0;
    for ( ; bytes_received < data_length; ++bytes_received)
    {
        uint8_t value = 0;
        uint8_t mix = 0;
        for (uint8_t bitMask = 0x01; bitMask != 0; bitMask <<= 1)
        {
            if (recvBit())
            {
                value |= bitMask;
                mix = 1;
            }
            else mix = 0;

            if (_error != Error::NO_ERROR)
            {
                if ((bitMask == 0x01) && (_error ==Error::AWAIT_TIMESLOT_TIMEOUT_HIGH)) _error = Error::FIRST_BIT_OF_BYTE_TIMEOUT;
                interrupts();
                return true;
            }

            mix ^= static_cast<uint8_t>(crc16) & static_cast<uint8_t>(0x01);
            crc16 >>= 1;
            if (mix != 0)  crc16 ^= static_cast<uint16_t>(0xA001);
        }

        address[bytes_received] = value;
    }

    interrupts();
    return (bytes_received != data_length);
}


void wait(const uint16_t timeout_us)
{
    uint32_t loops = timeUsToLoops(timeout_us);
    bool state = false;
    while (loops != 0)
    {
        loops = waitLoopsWhilePinIs(loops,state);
        state = !state;
    }
}

void wait(const uint32_t loops_wait)
{
    uint32_t loops = loops_wait;
    bool state = false;
    while (loops != 0)
    {
        loops = waitLoopsWhilePinIs(loops,state);
        state = !state;
    }
}


// returns false if pins stays in the wanted state all the time
uint32_t waitLoopsWhilePinIs(volatile uint32_t retries, const bool pin_value)
{
    if (retries == 0) return 0;
    while ((DIRECT_READ(pin_baseReg, pin_bitMask) == pin_value) && (--retries != 0));
    return retries;
}



void raiseSlaveError(const uint8_t cmd)
{
    _error = Error::INCORRECT_SLAVE_USAGE;
}

void duty()
{
    uint8_t  reg_TA[2], cmd, data, crc = 0; // Target address, redirected address, command, data, crc

    if (recv(&cmd))  return;
    crc = crc8(&cmd,1,crc);

    if (recv(reg_TA,2))  return;
    crc = crc8(reg_TA,2,crc);

    if (reg_TA[1] != 0) return; // upper byte of target adress should not contain any data

    switch (cmd)
    {
        case 0xF0:      // READ MEMORY

            if (send(&crc))        break;

            crc = 0; // reInit CRC and send data
            for (uint8_t i = reg_TA[0]; i < sizeof_memory; ++i)
            {
                const uint8_t reg_RA = translateRedirection(i);
                if (send(&memory[reg_RA])) return;
                crc = crc8(&memory[reg_RA],1,crc);
            }
            send(&crc);
            break; // datasheet says we should return all 1s, send(255), till resetADS, nothing to do here, 1s are passive

        case 0xC3:      // READ DATA (like 0xF0, but repeatedly till the end of page with following CRC)

            if (send(&crc)) break;

            while (reg_TA[0] < sizeof_memory)
            {
                crc = 0; // reInit CRC and send data
                const uint8_t reg_EA = (reg_TA[0] & ~PAGE_MASK) + PAGE_SIZE; // End Address
                for (uint8_t i = reg_TA[0]; i < reg_EA; ++i)
                {
                    const uint8_t reg_RA = translateRedirection(i);
                    if (send(&memory[reg_RA])) return;
                    crc = crc8(&memory[reg_RA], 1, crc);
                }

                if (send(&crc)) break;
                reg_TA[0] = reg_EA;
            }
            break; // datasheet says we should return all 1s, send(255), till resetADS, nothing to do here, 1s are passive

        case 0xAA:      // READ STATUS // TODO: nearly same code as 0xF0, but with status[] instead of memory[]

            if (send(&crc)) break;

            crc = 0; // reInit CRC and send data
            for (uint8_t i = reg_TA[0]; i < STATUS_SIZE; ++i)
            {
                if (send(&status[i])) return;
                crc = crc8(&status[i],1,crc);
            }
            send(&crc);
            break; // datasheet says we should return all 1s, send(255), till resetADS, nothing to do here, 1s are passive

        case 0x0F:      // WRITE MEMORY

            while (reg_TA[0] < sizeof_memory)
            {
                if (recv(&data))       break;
                crc = crc8(&data,1,crc);

                if (send(&crc))        break;

                const uint8_t reg_RA = translateRedirection(reg_TA[0]);

                if (getPageProtection(reg_TA[0]))
                {
                    const uint8_t mem_zero = 0x00; // send dummy data
                    if (send(&mem_zero)) break;
                }
                else
                {
                    memory[reg_RA] &= data; // EPROM-Mode
                    setPageUsed(reg_RA);
                    if (send(&memory[reg_RA])) break;
                }
                crc = ++reg_TA[0];
            }
            break;

        case 0x55:      // WRITE STATUS

            while (reg_TA[0] < STATUS_SIZE)
            {
                if (recv(&data))       break;
                crc = crc8(&data,1,crc);

                if (send(&crc))        break;

                data = writeStatus(reg_TA[0], data);

                if (send(&data)) break;

                crc = ++reg_TA[0];
            }
            break;

        default:

            raiseSlaveError(cmd);
    }
}

uint8_t translateRedirection(const uint8_t source_address)
{
    const uint8_t  source_page    = static_cast<uint8_t >(source_address >> 5);
    const uint8_t  destin_page    = getPageRedirection(source_page);
    if (destin_page == 0x00)        return source_address;
    const uint8_t destin_address  = (source_address & PAGE_MASK) | (destin_page << 5);
    return destin_address;
}


void clearStatus(void)
{
    memset(status, static_cast<uint8_t>(0xFF), STATUS_SIZE);
    status[STATUS_FACTORYP] = 0x00; // last byte should be always zero
}

void clearMemory(void)
{
    memset(memory, static_cast<uint8_t>(0xFF), MEM_SIZE);
}

bool writeDS5202Memory(const uint8_t *source, uint8_t length, uint8_t position)
{
    if (position >= MEM_SIZE) return false;
    const uint16_t _length = (position + length >= MEM_SIZE) ? (MEM_SIZE - position) : length;
    memcpy(&memory[position],source,_length);

    const uint8_t page_start = static_cast<uint8_t>(position >> 5);
    const uint8_t page_stop  = static_cast<uint8_t>((position + _length) >> 5);
    for (uint8_t page = page_start; page <= page_stop; page++) setPageUsed(page);

    return (_length==length);
}



uint8_t writeStatus(const uint8_t address, const uint8_t value)
{
    if (address < STATUS_UNDEF_B1)  status[address] &= value; // writing is allowed only here
    return status[address];
}

void setPageProtection(const uint8_t page)
{
    if (page < PAGE_COUNT)          status[STATUS_WP_PAGES] &= ~(uint8_t(1<<page));
}

bool getPageProtection(const uint8_t page)
{
    if (page >= PAGE_COUNT) return true;
    return ((status[STATUS_WP_PAGES] & uint8_t(1<<page)) == 0);
}

void setPageUsed(const uint8_t page)
{
    if (page < PAGE_COUNT)  status[STATUS_WP_PAGES] &= ~(uint8_t(1<<(page+4)));
}

bool getPageUsed(const uint8_t page)
{
    if (page >= PAGE_COUNT) return true;
    return ((status[STATUS_WP_PAGES] & uint8_t(1<<(page+4))) == 0);
}


bool setPageRedirection(const uint8_t page_source, const uint8_t page_destin)
{
    if (page_source >= PAGE_COUNT)  return false; // really available
    if (page_destin >= PAGE_COUNT)  return false; // virtual mem of the device

    status[page_source + STATUS_PG_REDIR] = (page_destin == page_source) ? uint8_t(0xFF) : ~page_destin; // datasheet dictates this, so no page can be redirected to page 0
    return true;
}

uint8_t getPageRedirection(const uint8_t page)
{
    if (page >= PAGE_COUNT) return 0x00;
    return ~(status[page + STATUS_PG_REDIR]); // TODO: maybe invert this in ReadStatus and safe some Operations? Redirection is critical and often done
}


#if defined(__AVR__)
#include <util/crc16.h>
#endif


uint8_t crc8(const uint8_t data[], const uint8_t data_size, const uint8_t crc_init)
{
    uint8_t crc = crc_init;

    for (uint8_t index = 0; index < data_size; ++index)
    {
#if defined(__AVR__)
        crc = _crc_ibutton_update(crc, data[index]);
#else
        uint8_t inByte = data[index];
        for (uint8_t bitPosition = 0; bitPosition < 8; ++bitPosition)
        {
            const uint8_t mix = (crc ^ inByte) & static_cast<uint8_t>(0x01);
            crc >>= 1;
            if (mix != 0) crc ^= 0x8C;
            inByte >>= 1;
        }
#endif
    }
    return crc;
}


uint16_t crc16(const uint8_t address[], const uint8_t length, const uint16_t init)
{
    uint16_t crc = init; // init value

#if defined(__AVR__)
    for (uint8_t i = 0; i < length; ++i)
    {
        crc = _crc16_update(crc, address[i]);
    }
#else
    static const uint8_t oddParity[16] =
            {0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0};

    for (uint8_t i = 0; i < length; ++i)
    {
        // Even though we're just copying a byte from the input,
        // we'll be doing 16-bit computation with it.
        uint16_t cdata = address[i];
        cdata = (cdata ^ crc) & static_cast<uint16_t>(0xff);
        crc >>= 8;

        if ((oddParity[cdata & 0x0F] ^ oddParity[cdata >> 4]) != 0)
            crc ^= 0xC001;

        cdata <<= 6;
        crc ^= cdata;
        cdata <<= 1;
        crc ^= cdata;
    }
#endif
    return crc;
}

uint16_t crc16(uint8_t value, uint16_t crc)
{
#if defined(__AVR__)
    return _crc16_update(crc, value);
#else
    static const uint8_t oddParity[16] = {0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0};
    value = (value ^ static_cast<uint8_t>(crc));
    crc >>= 8;
    if ((oddParity[value & 0x0F] ^ oddParity[value >> 4]) != 0)   crc ^= 0xC001;
    uint16_t cdata = (static_cast<uint16_t>(value) << 6);
    crc ^= cdata;
    crc ^= (static_cast<uint16_t>(cdata) << 1);
    return crc;
#endif
}
