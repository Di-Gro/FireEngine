using System;

[Flags]
public enum Flag : ulong {
    Null = 0,
    Player = 0b1,
    TestFlag2 = 0b10,
    TestFlag3 = 0b100,
    TestFlag4 = 0b1000,
    TestFlag5 = 0b10000,

}