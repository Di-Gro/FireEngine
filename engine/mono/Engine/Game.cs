using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;


namespace Engine {

    static class Game {

        public static CppRef gameRef { get; private set; }
        public static CppRef meshAssetRef { get; private set; }

        private static void cpp_SetGameRef(CppRef value) {
            Console.WriteLine($"#: Game.cpp_SetGameRef(): {value}");

            gameRef = value;
        }

        private static void cpp_SetMeshAssetRef(CppRef value) {
            Console.WriteLine($"#: Game.cpp_SetMeshAssetRef(): {value}");

            meshAssetRef = value;
        }

    }
}
