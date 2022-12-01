using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;


namespace Engine {

    static class Game {

        public static CppRef gameRef { get; private set; }
        public static CppRef meshAssetRef { get; private set; }
        public static float DeltaTime => m_updateData.deltaTime;


        private static GameUpdateData m_updateData;


        private static void cpp_SetGameRef(CppRef value) {
            Console.WriteLine($"#: Game.cpp_SetGameRef(): {value}");

            gameRef = value;
        }

        private static void cpp_SetMeshAssetRef(CppRef value) {
            Console.WriteLine($"#: Game.cpp_SetMeshAssetRef(): {value}");

            meshAssetRef = value;
        }

        private static void cpp_SetUpdateData(GameUpdateData value) {
            //Console.WriteLine($"#: Game.cpp_SetUpdateData(): {value.deltaTime}");
            m_updateData = value;
        }

    }

    [StructLayout(LayoutKind.Sequential)]
    struct GameUpdateData {
        public float deltaTime;
    };
}
