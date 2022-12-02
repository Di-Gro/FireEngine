using System;
using System.IO;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

using EngineDll;


namespace Engine {

    class Mesh {
        public CppRef meshRef { get; protected set; }

        public int ShapeCount => Dll.Mesh4.ShapeCount(meshRef);
        public int MaxMaterialIndex => Dll.Mesh4.MaterialMaxIndex(meshRef);


        public Mesh() { }
        public Mesh(CppRef cppRef) => meshRef = cppRef;

    }

    class StaticMesh : Mesh {

        private ulong m_assetHash;


        public StaticMesh(string file) {
            m_assetHash = Dll.MeshAsset.CreateHash(Game.meshAssetRef, file);
            Dll.MeshAsset.Load(Game.meshAssetRef, m_assetHash);

            meshRef = Dll.MeshAsset.GetMesh(Game.meshAssetRef, m_assetHash);
        }

    }

}
