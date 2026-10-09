#include "uart/uart.h"

Uart::Uart(HardwareSerial& serial) : serial_(serial) {}

void Uart::begin(uint32_t baud) {
    serial_.begin(baud);
}

size_t Uart::read(uint8_t* buf, size_t max) {
    size_t n = 0;
    while (n < max && serial_.available() > 0) {
        buf[n++] = static_cast<uint8_t>(serial_.read());
    }
    return n;
}
