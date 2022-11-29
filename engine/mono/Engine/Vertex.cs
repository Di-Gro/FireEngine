using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;


namespace Engine {

    [StructLayout(LayoutKind.Sequential)]
    struct Vertex {
        public Vector4 position;
        public Vector4 color;
        public Vector4 normal;
        public Vector4 uv;

        public static Vertex Default = new Vertex() {
            position = Vector4.Zero,
            color = Vector4.One,
            normal = Vector4.One,
            uv = Vector4.Zero
        };
    }
}
