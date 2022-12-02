using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

using EngineDll;

namespace Engine {

    interface IMaterial {
        CppRef matRef { get; }

        string Name { get; }

        Vector3 DiffuseColor { get; }
        float Diffuse { get; }
        float Ambient { get; }
        float Specular { get; }
        float Shininess { get; }
    } 

    class StaticMaterial : IMaterial {

        public CppRef matRef { get; protected set; }

        public string Name { get; private set; }
        
        public Vector3 DiffuseColor => Dll.Material.diffuseColor_get(matRef);
        public float Diffuse => Dll.Material.diffuse_get(matRef);
        public float Ambient => Dll.Material.ambient_get(matRef);
        public float Specular => Dll.Material.specular_get(matRef);
        public float Shininess => Dll.Material.shininess_get(matRef);


        public StaticMaterial(CppRef cppRef) {
            //Console.WriteLine($"#: new StaticMaterial({cppRef})");
            matRef = cppRef;

            byte[] buf = new byte[80];
            int writedLength = Dll.Material.name_get(matRef, buf, buf.Length);

            Name = Encoding.ASCII.GetString(buf);
        }

    }

    class DynamicMaterial : IMaterial {

        public CppRef matRef { get; protected set; }

        public string Name { get; private set; }

        public Vector3 DiffuseColor { get => Dll.Material.diffuseColor_get(matRef); set => Dll.Material.diffuseColor_set(matRef, value); }
        public float Diffuse { get => Dll.Material.diffuse_get(matRef); set => Dll.Material.diffuse_set(matRef, value); }
        public float Ambient { get => Dll.Material.ambient_get(matRef); set => Dll.Material.ambient_set(matRef, value); }
        public float Specular { get => Dll.Material.specular_get(matRef); set => Dll.Material.specular_set(matRef, value); }
        public float Shininess { get => Dll.Material.shininess_get(matRef); set => Dll.Material.shininess_set(matRef, value); }

        public bool IsDeleted => Ref.IsValid(matRef);


        public DynamicMaterial(IMaterial source) {
            //Console.WriteLine($"#: new DynamicMaterial(source:{source.matRef})");

            matRef = Dll.MeshAsset.CreateDynamicMaterial(Game.meshAssetRef, source.matRef);

            byte[] buf = new byte[80];
            Dll.Material.name_get(matRef, buf, buf.Length);

            Name = Encoding.ASCII.GetString(buf);
        }

        public void Delete() {
            Dll.MeshAsset.DeleteDynamicMaterial(Game.meshAssetRef, matRef);
        }

    }

}
