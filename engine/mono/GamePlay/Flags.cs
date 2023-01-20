using System;

[Flags]
public enum Flag : ulong {
    Null = 0,
    Player = 0b1,
    Enemy = 0b10,
    IsItem = 0b100,
    EMPTY_1 = 0b1000,
    IsRetarget = 0b10000,
    IsDropTarget = 0b100000,
}