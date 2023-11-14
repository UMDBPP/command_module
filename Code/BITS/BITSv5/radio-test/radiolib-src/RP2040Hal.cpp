#include "RP2040Hal.h"

#include "hardware/gpio.h"
#include "pico/stdlib.h"

RP2040Hal::RP2040Hal()
    : RadioLibHal(GPIO_IN, GPIO_OUT, 0, 1, 3, 4), initInterface(true) {}

void RP2040Hal::init() {
    if (initInterface) {
        spi_init(spi0, 2000000);
    }
}

void RP2040Hal::term() {
    if (initInterface) {
        spi_deinit(spi0);
    }
}

void inline RP2040Hal::pinMode(uint32_t pin, uint32_t mode) {
    if (pin == RADIOLIB_NC) {
        return;
    }

    if (mode == GPIO_OUT) {
        gpio_set_dir(pin, GPIO_OUT);
    } else {
        gpio_set_dir(pin, GPIO_IN);
    }
}

void inline RP2040Hal::digitalWrite(uint32_t pin, uint32_t value) {
    if (pin == RADIOLIB_NC) {
        return;
    }

    if (value == 0) {
        gpio_put(pin, 0);
    } else {
        gpio_put(pin, 1);
    }
}

uint32_t inline RP2040Hal::digitalRead(uint32_t pin) {
    if (pin == RADIOLIB_NC) {
        return 0;
    }
    return gpio_get(pin);
}

void inline RP2040Hal::attachInterrupt(uint32_t interruptNum,
                                       void (*interruptCb)(void),
                                       uint32_t mode) {
    if (interruptNum == RADIOLIB_NC) {
        return;
    }

    gpio_set_irq_enabled_with_callback(interruptNum,
                                       GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL,
                                       true, interruptCb);
}

void inline RP2040Hal::detachInterrupt(uint32_t interruptNum) {
    if (interruptNum == RADIOLIB_NC) {
        return;
    }
    gpio_set_irq_enabled(interruptNum, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL,
                         false);
}

void inline RP2040Hal::delay(unsigned long ms) { sleep_ms(ms); }

void inline RP2040Hal::delayMicroseconds(unsigned long us) { sleep_us(us); }

unsigned long inline RP2040Hal::millis() {
    return (unsigned long)time_us_64 / 1000;
}

unsigned long inline RP2040Hal::micros() { return (unsigned long)time_us_64; }

long inline RP2040Hal::pulseIn(uint32_t pin, uint32_t state,
                               unsigned long timeout) {
    if (pin == RADIOLIB_NC) {
        return 0;
    }
    return (::pulseIn(pin, state, timeout));
}

void inline RP2040Hal::spiBegin() {}

void inline RP2040Hal::spiBeginTransaction() {}

void RP2040Hal::spiTransfer(uint8_t* out, size_t len, uint8_t* in) {
    for (size_t i = 0; i < len; i++) {
        in[i] = spi->transfer(out[i]);
    }
}

void inline RP2040Hal::spiEndTransaction() { spi->endTransaction(); }

void inline RP2040Hal::spiEnd() { spi->end(); }

void RP2040Hal::readPersistentStorage(uint32_t addr, uint8_t* buff,
                                      size_t len) {}

void RP2040Hal::writePersistentStorage(uint32_t addr, uint8_t* buff,
                                       size_t len) {}

void inline RP2040Hal::tone(uint32_t pin, unsigned int frequency,
                            unsigned long duration) {
#if !defined(RADIOLIB_TONE_UNSUPPORTED)
    if (pin == RADIOLIB_NC) {
        return;
    }
    ::tone(pin, frequency, duration);
#elif defined(RADIOLIB_ESP32)
    // ESP32 tone() emulation
    (void)duration;
    if (prev == -1) {
#if !defined(ESP_IDF_VERSION) || \
    (ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0))
        ledcAttachPin(pin, RADIOLIB_TONE_ESP32_CHANNEL);
#endif
    }
    if (prev != frequency) {
#if !defined(ESP_IDF_VERSION) || \
    (ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0))
        ledcWriteTone(RADIOLIB_TONE_ESP32_CHANNEL, frequency);
#else
        ledcWriteTone(pin, frequency);
#endif
    }
    prev = frequency;
#elif defined(RADIOLIB_MBED_TONE_OVERRIDE)
    // better tone for mbed OS boards
    (void)duration;
    if (!pwmPin) {
        pwmPin = new mbed::PwmOut(digitalPinToPinName(pin));
    }
    pwmPin->period(1.0 / frequency);
    pwmPin->write(0.5);
#endif
}

void inline RP2040Hal::noTone(uint32_t pin) {
#if !defined(RADIOLIB_TONE_UNSUPPORTED) and defined(ARDUINO_ARCH_STM32)
    if (pin == RADIOLIB_NC) {
        return;
    }
    ::noTone(pin, false);
#elif !defined(RADIOLIB_TONE_UNSUPPORTED)
    if (pin == RADIOLIB_NC) {
        return;
    }
    ::noTone(pin);
#elif defined(RADIOLIB_ESP32)
    if (pin == RADIOLIB_NC) {
        return;
    }
// ESP32 tone() emulation
#if !defined(ESP_IDF_VERSION) || \
    (ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0))
    ledcDetachPin(pin);
    ledcWrite(RADIOLIB_TONE_ESP32_CHANNEL, 0);
#else
    ledcDetach(pin);
    ledcWrite(pin, 0);
#endif
    prev = -1;
#elif defined(RADIOLIB_MBED_TONE_OVERRIDE)
    if (pin == RADIOLIB_NC) {
        return;
    }
    // better tone for mbed OS boards
    (void)pin;
    pwmPin->suspend();
#endif
}

void inline RP2040Hal::yield() {
#if !defined(RADIOLIB_YIELD_UNSUPPORTED)
    ::yield();
#endif
}

uint32_t inline RP2040Hal::pinToInterrupt(uint32_t pin) {
    return (digitalPinToInterrupt(pin));
}