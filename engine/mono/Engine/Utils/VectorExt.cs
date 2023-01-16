
using Engine;

public static class Vector3Ext {

    public static Vector3 Normalized(this Vector3 v) {
        float length = v.Length();
        return length == 0 ? Vector3.Zero : v / v.Length();
    }

} 
