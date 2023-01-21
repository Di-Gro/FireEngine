using System;

[Flags]
public enum Flag : ulong {
    Null = 0,
    Player = 1 << 0,
    Enemy = 1 << 1,
    IsItem = 1 << 2,
    EMPTY_1 = 1 << 3,
    IsRetarget = 1 << 4,
    IsDropTarget = 1 << 5,
    NavMesh = 1 << 6,
    IsBullet = 1 << 7,
}