using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

namespace Engine {

    [StructLayout(LayoutKind.Sequential)]
    public struct Contact {
        public Vector3 point1;
        public Vector3 point2;
        public Vector3 relativePoint1;
        public Vector3 relativePoint2;
        public Vector3 normal;
        public Vector3 offset;
        public float depth;
    }
}