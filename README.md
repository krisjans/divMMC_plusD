# Access Flopy disks using PlusD-like device connected to divIDE/divMMC;

Following hardware modifications to PlusD is requred:
 * remove RAM and ROM chips from plusD;
 * disconnect plusD zx-connector pins A25, B4 and A15 (those pins are used to swap-in RAM/ROM on plusD into zx memory-space, not needed because divMMC RAM/ROM will be used)
 * swap zx-connector pins A11 and A22 (this swaps bus address lines A2 and A6);

 To build source code you need z88dk version v2.3 or later;
