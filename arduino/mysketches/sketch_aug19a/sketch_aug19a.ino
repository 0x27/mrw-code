// This example is based in JCW's KAKU RF sockets control code
// 2009-02-21 jc@wippler.nl http://opensource.org/licenses/mit-license.php
 
// Note that 868 MHz RFM12B's can send 433 MHz just fine, even though the RF
// circuitry is presumably not optimized for that band. Maybe the range will
// be limited, or maybe it's just because 868 is nearly a multiple of 433 ?
 
#include <JeeLib.h>
#include <util/parity.h>
 
// Turn transmitter on or off, but also apply asymmetric correction and account
// for 25 us SPI overhead to end up with the proper on-the-air pulse widths.
// With thanks to JGJ Veken for his help in getting these values right.
static void ookPulse(int on, int off) {
    rf12_onOff(1);
    delayMicroseconds(on + 150);
    rf12_onOff(0);
    delayMicroseconds(off - 200);
}

#define ON 1125
#define OFF 375
#define TOTAL 1500
 
static void OOKSend(unsigned long cmd) {
    for (byte i = 0; i < 4; ++i) {
        for (byte bit = 0; bit < 18; ++bit) {
            ookPulse(OFF, ON);
            int on = bitRead(cmd, bit) ? ON : OFF;
            ookPulse(on, TOTAL - on);
        }
        ookPulse(375, 375);
        delay(11); // approximate
    }
}
 
void setup() {
    Serial.begin(9600);
    Serial.println("\n[OOK_RF sockets]");
    rf12_initialize(0, RF12_433MHZ);
    
    //rf12_control(0x8017); // 8027    868 Mhz;disabel tx register; disable RX
                          //         fifo buffer; xtal cap 12pf, same as xmitter
    //rf12_control(0x82FD); // 82C0    enable receiver; enable basebandblock 
    rf12_control(0xA620); // A68A    433.92mhz (A620) or 434mhz (AC40)
    rf12_control(0xc647); // C691    c691 datarate 2395 kbps 0xc647 = 4.8kbps 
    //rf12_control(0x948A); // 9489    VDI; FAST;200khz;GAIn -6db; DRSSI 97dbm   (948A for long range, 948B for short range < 1m)
    //rf12_control(0xC220); // C220    datafiltercommand; ** not documented cmd 
    //rf12_control(0xCA00); // CA00    FiFo and resetmode cmd; FIFO fill disabeld
    //rf12_control(0xC473); // C473    AFC run only once; enable AFC; enable
                          //         frequency offset register; +3 -4
    //rf12_control(0xCC67); // CC67    pll settings command
    //rf12_control(0xB800); // TX register write command not used
    //rf12_control(0xC800); // disable low dutycycle 
    //rf12_control(0xC040); // 1.66MHz,2.2V not used see 82c0
}
 
void loop() {
    delay(5000);
    Serial.println("Start pulses");
    //OOKSend(0b0011000000001110);
    OOKSend(0b0000111000110000);
    Serial.println("End pulses");
    delay(5000);
    Serial.println("Start pulses");
    OOKSend(0b0000011000110000);
    Serial.println("End pulses");
    //delay(2000);
}
