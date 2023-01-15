# The assembler currently assumes that the top of assembly files corresponds with memory location 0x00.
# This will be configurable later on as having all these HLTs just to hit the 0x10 reset vector is a pain.
HLT
HLT
HLT
HLT
HLT
HLT
HLT
HLT
HLT
HLT
HLT
HLT
HLT
HLT
HLT
HLT
LDB R1, 0x11
LDB R2, 0x22
ADD R1, R1, R2
STR R1, 0xFF