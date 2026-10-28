#include "drivers/rfm95/rfm95.h"

#include <Arduino.h>
#include <cstdio>
#include <type_traits>

namespace {

constexpr uint8_t kRstPin = 9;

// Registers (SX1276 datasheet, LoRa register map)
constexpr uint8_t kRegFifo = 0x00;
constexpr uint8_t kRegOpMode = 0x01;
constexpr uint8_t kRegFrfMsb = 0x06;  // then 0x07 Mid, 0x08 Lsb
constexpr uint8_t kRegPaConfig = 0x09;
constexpr uint8_t kRegFifoAddrPtr = 0x0D;
constexpr uint8_t kRegFifoTxBaseAddr = 0x0E;
constexpr uint8_t kRegModemConfig2 = 0x1E;
constexpr uint8_t kRegPayloadLength = 0x22;
constexpr uint8_t kRegVersion = 0x42;

constexpr uint8_t kVersion = 0x12;

// RegOpMode: bit 7 selects LoRa, bits 2-0 the mode
constexpr uint8_t kModeLoRa = 0x80;
constexpr uint8_t kModeSleep = 0x00;
constexpr uint8_t kModeStandby = 0x01;
constexpr uint8_t kModeTx = 0x03;
constexpr uint8_t kModeMask = 0x07;

// Frf = freq * 2^19 / 32 MHz = 915 MHz -> 0xE4C000
constexpr uint8_t kFrf[3] = {0xE4, 0xC0, 0x00};

// PA_BOOST (the only PA wired on the RFM95), Pout = 2 + 15 = 17 dBm
constexpr uint8_t kPaConfig = 0x8F;

// SF7 (the default) with the CRC-on bit set
constexpr uint8_t kModemConfig2 = 0x74;

constexpr size_t kMaxPayload = 255;

// Write one value as text, snprintf-style: returns the length it needed
int formatValue(char* out, size_t cap, const Value& value) {
    return std::visit(
        [&](const auto& v) -> int {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<T, std::string>) {
                return snprintf(out, cap, "%s", v.c_str());
            } else if constexpr (std::is_same_v<T, uint32_t>) {
                return snprintf(out, cap, "%lu", static_cast<unsigned long>(v));
            } else if constexpr (std::is_same_v<T, bool> || std::is_same_v<T, int>) {
                return snprintf(out, cap, "%d", static_cast<int>(v));
            } else {
                return snprintf(out, cap, "%.3f", static_cast<double>(v));
            }
        },
        value);
}

// Pack channels as "name=value,name=value" into `out`, stopping at the
// first channel that doesn't fit. Returns the packed length.
size_t packChannels(const std::deque<Channel>& channels, char* out, size_t cap) {
    size_t len = 0;
    for (const Channel& ch : channels) {
        size_t start = len;
        int n = snprintf(out + len, cap - len, "%s%s=", len ? "," : "", ch.name.c_str());
        if (n < 0 || static_cast<size_t>(n) >= cap - len) return start;
        len += n;
        n = formatValue(out + len, cap - len, ch.value);
        if (n < 0 || static_cast<size_t>(n) >= cap - len) return start;
        len += n;
    }
    return len;
}

}  // namespace

Rfm95::Rfm95(Blackboard& bb, Spi& spi)
    : bb_(bb),
      spi_(spi),
      healthy_(bb.registerChannel<bool>("radio.healthy")),
      packets_sent_(bb.registerChannel<int>("radio.packets_sent")) {}

bool Rfm95::init() {
    // Pulse reset low; the radio is ready ~5 ms after release
    pinMode(kRstPin, OUTPUT);
    digitalWrite(kRstPin, LOW);
    delay(10);
    digitalWrite(kRstPin, HIGH);
    delay(10);

    spi_.begin();
    if (spi_.read(kRegVersion) != kVersion) return false;

    // The LoRa bit can only be changed in sleep mode
    spi_.write(kRegOpMode, kModeSleep);
    spi_.write(kRegOpMode, kModeLoRa | kModeSleep);

    spi_.write(kRegFrfMsb, kFrf, sizeof kFrf);
    spi_.write(kRegPaConfig, kPaConfig);
    spi_.write(kRegModemConfig2, kModemConfig2);
    // Start TX data at the bottom of the FIFO so the whole 256 bytes is usable
    spi_.write(kRegFifoTxBaseAddr, 0x00);

    spi_.write(kRegOpMode, kModeLoRa | kModeStandby);
    return true;
}

void Rfm95::dispatch() {
    // A missing radio reads 0x00 or 0xFF, neither of which looks like LoRa
    // mode with the other high bits clear
    uint8_t mode = spi_.read(kRegOpMode);
    healthy_ = (mode & ~kModeMask) == kModeLoRa;
    if (!healthy_) return;

    // The radio drops back to standby by itself once a packet is sent
    if ((mode & kModeMask) == kModeTx) return;

    char buf[kMaxPayload + 1];  // +1 for snprintf's terminator, not sent
    size_t len = packChannels(bb_.channels(), buf, sizeof buf);

    spi_.write(kRegFifoAddrPtr, 0x00);
    spi_.write(kRegFifo, reinterpret_cast<const uint8_t*>(buf), len);
    spi_.write(kRegPayloadLength, static_cast<uint8_t>(len));
    spi_.write(kRegOpMode, kModeLoRa | kModeTx);
    packets_sent_++;
}
