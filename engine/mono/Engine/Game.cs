using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

using EngineDll;

namespace Engine {

    static class Game {

        public static CppRef gameRef { get; private set; }
        public static CppRef meshAssetRef { get; private set; }
        public static CppRef sceneRef { get; private set; }
        public static float DeltaTime => m_updateData.deltaTime;


        private static GameUpdateData m_updateData;
        private static GameCallbacks m_gameCallbacks;


        public static List<Actor> GetRootActors() {
            int count = Dll.Game.GetRootActorsCount(sceneRef);
            var refs = new CsRef[count];

            Dll.Game.WriteRootActorsRefs(sceneRef, refs);

            var result = new List<Actor>(count);
            foreach(var objRef in refs) {
                var actor = CppLinked.GetObjectByRef(objRef) as Actor;
                result.Add(actor);
            }
            return result;
        }

        private static void cpp_Init(CppRef _gameRef) {
            gameRef = _gameRef;

            m_gameCallbacks = new GameCallbacks();
            m_gameCallbacks.setSceneRef = new GameCallbacks.SetRef(SetSceneRef);
            m_gameCallbacks.setMeshAssetRef = new GameCallbacks.SetRef(SetMeshAssetRef);
            m_gameCallbacks.setUpdateData = new GameCallbacks.SetUpdateData(SetUpdateData);

            Dll.Game.SetGameCallbacks(Game.gameRef, m_gameCallbacks);
        }

        private static void SetSceneRef(CppRef value) {
            sceneRef = value;
        }

        private static void SetMeshAssetRef(CppRef value) {
            meshAssetRef = value;
        }

        private static void SetUpdateData(GameUpdateData value) {
            m_updateData = value;
        }

    }

    [StructLayout(LayoutKind.Sequential)]
    public struct GameUpdateData {
        public float deltaTime;
    };

    [StructLayout(LayoutKind.Sequential)]
    public struct GameCallbacks {
        public delegate void SetRef(CppRef value);
        public delegate void SetUpdateData(GameUpdateData value);

        public SetRef setSceneRef;
        public SetRef setMeshAssetRef;
        public SetUpdateData setUpdateData;
    }
}


