using System;
using System.IO;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

using EngineMono;


namespace Engine {

    class Mesh {
        public CppRef meshRef { get; protected set; }

        //public string Name { get; protected set; }

        public int ShapeCount => dll_ShapeCount(meshRef);
        public int MaxMaterialIndex => dll_MaxMaterialIndex(meshRef);


        public Mesh() { }
        public Mesh(CppRef cppRef) => meshRef = cppRef;


        [DllImport(MonoClass.ExePath, EntryPoint = "Mesh4_ShapeCount")]
        private static extern int dll_ShapeCount(CppRef meshAssetRef);

        [DllImport(MonoClass.ExePath, EntryPoint = "Mesh4_MaxMaterialIndex")]
        private static extern int dll_MaxMaterialIndex(CppRef meshAssetRef);
    }

    class StaticMesh : Mesh {

        private ulong m_assetHash;


        public StaticMesh(string file) {
            m_assetHash = dll_CreateHash(Game.meshAssetRef, file);
            dll_Load(Game.meshAssetRef, m_assetHash);

            meshRef = dll_GetMesh(Game.meshAssetRef, m_assetHash);
            //Name = Path.GetFileNameWithoutExtension(file);

            //Console.WriteLine($"#: m_meshRef: {meshRef}");
        }


        [DllImport(MonoClass.ExePath, EntryPoint = "MeshAsset_CreateHash", CharSet = CharSet.Ansi)]
        private static extern ulong dll_CreateHash(CppRef meshAssetRef, string str);

        [DllImport(MonoClass.ExePath, EntryPoint = "MeshAsset_Load")]
        private static extern void dll_Load(CppRef meshAssetRef, ulong hash);

        [DllImport(MonoClass.ExePath, EntryPoint = "MeshAsset_GetMesh")]
        private static extern CppRef dll_GetMesh(CppRef meshAssetRef, ulong hash);

    }

    //class DynamicMesh : StaticMesh {
    //    public DynamicMesh(StaticMesh staticMesh) { }

    //    public void UpdateShape(int shapeIndex, Vector3[] vertexes) { }
    //}

    //class Material { 
    
    //}

}
