
## PCB placement, keys and encoders

**Encoders**: `EC[1..2]`
**Keys/Leds**: `01..30`
**Keys**: `RST`, `DFU`, `A..C`

```
                      EC1       EC2

                          RST   DFU

    01    02    03    04    05    06

A   07    08    09    10    11    12   B

    13    14    15    16    17    18   C

    19    20    21    22    23    24

    25    26    27    28    29    30
```

## Keys

|Key| GPIO | Key| GPIO | Key| GPIO |
|:-:|------|:--:|------|:--:|------|
|01 | D13  | 16 | C5   | A  | D12  |
|02 | C8   | 17 | D7   | B  | C1   |
|03 | B10  | 18 | E6   | C  | C0   |
|04 | B4   | 19 | C6   |    |      |
|05 | C13  | 20 | A10  |    |      |
|06 | C15  | 21 | C10  |    |      |
|07 | D14  | 22 | B0   |    |      |
|08 | C9   | 23 | B3   |    |      |
|09 | E15  | 24 | E5   |    |      |
|10 | A6   | 25 | C7   |    |      |
|11 | E4   | 26 | A9   |    |      |
|12 | C14  | 27 | A15  |    |      |
|13 | D15  | 28 | B1   |    |      |
|14 | A8   | 29 | D6   |    |      |
|15 | B11  | 30 | E3   |    |      |

## LEDs, charlieplexed

LEDS | GPIO 
:---:|------
  1  |  D0 
  2  |  D1
  3  |  D2
  4  |  D3
  5  |  D4
  6  |  D5    

## LCD

LCD  | GPIO | Peripheral | AF
-----|------|------------|----
LED  | B14  |            |
RST  | D8   |            |
RS   | D9   |            |
MOSI | B15  | SPI2_MOSI  | AF5
SCK  | B13  | SPI2_SCK   | AF5
EN   | D10  |            |
CS   | B12  | SPI2_NSS   | AF5

## DAC

To confirm sampling frequency, measure clock on DAC's BCK (13) pin

|Sample f(khz)| BCK MHz (64)|
|:--:|:------:|
| 16 | 1.024  |
| 32 | 2.048  |
| 44 | 2.8224 |
| 48 | 3.072  |
| 96 | 6.144  |
|192 | 12.288 |
|384 | 24.576 |

(full table on page 25 TI's SLAS859C –MAY 2012– REVISED MAY 2015)

