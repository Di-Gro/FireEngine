using System;

[Flags]
public enum Flag : ulong {
    Null = 0,
    Player = 1 << 1,
    Enemy = 1 << 2,
    IsItem = 1 << 3,
    EMPTY_1 = 1 << 4,
    IsRetarget = 1 << 5,
    IsDropTarget = 1 << 6,
    NavMesh = 1 << 7,
}