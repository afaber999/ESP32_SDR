# ESP32 SDR based SSB radio (transmit and receive)
SSB radio transceiver based on the ESP32-A1S, which contains an ESP32 with the ES8388 audio
codec. Many ideas and code is borrowed from the uSDR-pico project project https://github.com/ArjanteMarvelde/uSDR-pico

The idea is to build an SSB transceiver using the ESP32-A1S module (ESP32 with ES8388 Audio Codec) as the main processor (radio control, LCD based UI & DSP audio processing)
The project is highly experimental, foremost intended to investigate use the ESP32 and ES8388 within a transceiver project. At this point in time, the code base  contains the code for an experimental implementation of the control and signal processing for a QSD/QSE based transceiver.
In addition, I included the filter modeliing files and some spice simulation files..  

The software is distributed over the two cores: core-1 takes care of all user I/O and control functions, while core-0 performs all of the signal processing. The samples are (ADC) are acquired by core-0, the sampling process itself is taken care of by the DMA engine which are acquired via the I2S interface between the ES8388 and the ESP32. Currently I use blocks of 64 smples (per channel). This block is processed using the steps below:
 
The RX-branch
- takes latest I and Q samples from QSD on the ES8388 ADC input channel 2
- applies DC offset correction
- applies a FIR low-pass filter at Fc=3kHz, 
- do weaver demodulation to phase shift (and frequency shift)
- mix I and Q samples based on current RX mode
- scales, filters and outputs audio on the ES8388 DAC, towards audio output
The RX-branch
- tbd

The result is put into a 64 sample buffer as well and scheduled to be transmitted to the es8388 DAC using the I2S interface and the DMA engine of the ESP32. The sample rate is at this point in time fixed at 44.1 KHz. 

On core1 the main loop takes care of user I/O, all other controls and the monitor port. There is also a LED flashing timer callback functioning as a heartbeat.

The main loop controls an Si5351A clock module to obtain the switching clock for the QSE and QSD. The module outputs two synchronous square wave clocks on ch 0 and 1, whith selectable phase difference (0, 90, 180 or 270 degrees). The clock on ch2 is free to be used for other goals. The module is controlled over the **i2c1** channel.
The display is a standard 16x2 LCD, but with an I2C interface. The display is connected through the **i2c0** channel.

## Open issues: 
- [ ] Not even have an open item list yet

## Building ESP32_SDR firmware: 
Clone/copy the repositirt, e.g. **$SDR/ESP32_SDR**  and op project in VS Code with Platform.IO
Build and flash the firmware

## Misc: 

For calculating filters I have used the free software from Iowa Hills (http://www.iowahills.com/8DownloadPage.html)  
I also used the online FIR filter calculator T-Filter (http://t-filter.engineerjs.com/) 

# Copyright notice
The code and electronic designs as well as the implementations presented in this reposiory can be copied and modified freely, for non-commercial use.
Use for commercial purposes is allowed as well, as long as a reference to this repository is included in the product.


# STATUS
Currently the project is in an early prototyping stage, at this point in time 
working on the receiver part and getting to understand the ES8388 settings.

