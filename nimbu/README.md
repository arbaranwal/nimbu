| Supported Targets | ESP32 |
| ----------------- | ----- |

NIMBU
=====

Features:

* A2DP audio sink role: Advanced Audio Distribution Profile to receive an audio stream.
* AVRCP: for media information notifications
* I2S: audio stream output

## How to use

### Hardware Required

To play the sound, there is a need of loudspeaker and possibly an external I2S codec. Otherwise Nimbu will only show a count of audio data packets received silently. Internal DAC can be selected and in this case external I2S codec may not be needed.

For the I2S codec, pick whatever chip or board works for you; this code was written using a PCM5102 chip, but other I2S boards and chips will probably work as well. The default I2S connections are shown below, but these can be changed in menuconfig:

| ESP GPIO  | PERIPHERAL   | SIGNAL |
| :-------- | :----------- | :----- |
| GPIO22    | I2S          | LRCK   |
| GPIO25    | I2S          | DATA   |
| GPIO26    | I2S          | BCK    |
| GPIO17    | LED          | BLUE   |
| GPIO16    | LED          | GREEN  |
| GPIO4     | LED          | RED    |
| GPIO34    | ADC          | SRC1   |
| GPIO35    | ADC          | SRC2   |


If the internal DAC is selected, analog audio will be available on GPIO25 and GPIO26. The output resolution on these pins will always be limited to 8 bit because of the internal structure of the DACs.

## Understanding Output

After the program is started, Nimbu starts inquiry scan and page scan, awaiting being discovered and connected. Other bluetooth devices can discover a device named "nimbu".

Once A2DP connection is set up, there will be a notification message with the remote device's bluetooth MAC address like the following:

```
I (106427) BT_AV: A2DP connection state: Connected, [64:a2:f9:69:57:a4]
```

If a smartphone is used to connect to local device, starting to play music will result in the transmission of audio stream. The transmitting of audio stream will be visible in the application log including a count of audio data packets, like this:

```
I (120627) BT_AV: A2DP audio state: Started
I (122697) BT_AV: Audio packet count 100
I (124697) BT_AV: Audio packet count 200
I (126697) BT_AV: Audio packet count 300
I (128697) BT_AV: Audio packet count 400

```

Also, the sound will be heard if a loudspeaker is connected and possible external I2S codec is correctly configured. For ESP32 A2DP, the sound is noise as the audio source generates the samples with a random sequence.

## Troubleshooting
* For current stage, the supported audio codec in ESP32 A2DP is SBC. SBC data stream is transmitted to A2DP sink and then decoded into PCM samples as output. The PCM data format is normally of 44.1kHz sampling rate, two-channel 16-bit sample stream. Other decoder configurations in ESP32 A2DP sink is supported but need additional modifications of protocol stack settings.
* As a usage limitation, ESP32 A2DP sink can support at most one connection with remote A2DP source devices. Also, A2DP sink cannot be used together with A2DP source at the same time, but can be used with other profiles such as SPP and HFP.
