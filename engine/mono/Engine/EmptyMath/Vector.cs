//using System;
//using System.Collections.Generic;
//using System.Text;
//using System.Runtime.InteropServices;

//namespace Engine {

//    [StructLayout(LayoutKind.Sequential)]
//    struct Vector2 {
//        public float x;
//        public float y;

//        public Vector2(float x, float y) {
//            this.x = x;
//            this.y = y;
//        }

//        public Vector2(Vector3 vec) {
//            this.x = vec.x;
//            this.y = vec.y;
//        }

//        public override string ToString() {
//            return $"({x}, {y})";
//        }
//    }

//    [StructLayout(LayoutKind.Sequential)]
//    struct Vector3 {
//        public float x;
//        public float y;
//        public float z;

//        public Vector3(float x, float y, float z) {
//            this.x = x;
//            this.y = y;
//            this.z = z;
//        }

//        public override string ToString() {
//            return $"({x}, {y}, {z})";
//        }
//    }

//    [StructLayout(LayoutKind.Sequential)]
//    struct Vector4 {
//        public float x;
//        public float y;
//        public float z;
//        public float w;

//        public Vector4(float x, float y, float z, float w) {
//            this.x = x;
//            this.y = y;
//            this.z = z;
//            this.w = w;
//        }

//        public override string ToString() {
//            return $"({x}, {y}, {z}, {w})";
//        }

//        public static Vector4 Zero = new Vector4(0, 0, 0, 0);
//        public static Vector4 One = new Vector4(1, 1, 1, 1);
//    }
//}
