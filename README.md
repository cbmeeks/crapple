# crapple


| Start | End   | Description                                             |
|-------|-------|---------------------------------------------------------|
| $0000 | $00FF | Zero Page                                               |
| $0100 | $01FF | Stack                                                   |
| $0200 | $03FF | Mostly Free, Input Buffer, Interrupt Vectors            |
| $0400 | $07FF | Lo-res/Text Page1                                       |
| $0800 | $0BFF | Lo-res/Text Page2 (BASIC programs load here)            |
| $0C00 | $1FFF | Free                                                    |
| $2000 | $3FFF | Hi-res Page1                                            |
| $4000 | $5FFF | Hi-res Page2                                            |
| $6000 | $95FF | Free                                                    |
| $9600 | $BFFF | DOS3.3 and Buffers                                      |
| $C000 | $CFFF | Soft Switches, Expansion Card I/O and ROM               |
| $D000 | $F7FF | BASIC ROM (can be bankswitched later models)            |
| $F800 | $FFFF | Machine Language Monitor ROM (also can be bankswitched) |

