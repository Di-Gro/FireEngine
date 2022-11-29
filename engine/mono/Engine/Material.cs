using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

using EngineMono;

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
        
        public Vector3 DiffuseColor => MatDll.dll_diffuseColor_get(matRef);
        public float Diffuse => MatDll.dll_diffuse_get(matRef);
        public float Ambient => MatDll.dll_ambient_get(matRef);
        public float Specular => MatDll.dll_specular_get(matRef);
        public float Shininess => MatDll.dll_shininess_get(matRef);


        public StaticMaterial(CppRef cppRef) {
            Console.WriteLine($"#: new StaticMaterial({cppRef})");
            matRef = cppRef;

            byte[] buf = new byte[80];
            int writedLength = MatDll.dll_name_get(matRef, buf, buf.Length);

            Name = Encoding.ASCII.GetString(buf);
        }

    }

    class DynamicMaterial : IMaterial {

        public CppRef matRef { get; protected set; }

        public string Name { get; private set; }

        public Vector3 DiffuseColor { get => MatDll.dll_diffuseColor_get(matRef); set => MatDll.dll_diffuseColor_set(matRef, value); }
        public float Diffuse { get => MatDll.dll_diffuse_get(matRef); set => MatDll.dll_diffuse_set(matRef, value); }
        public float Ambient { get => MatDll.dll_ambient_get(matRef); set => MatDll.dll_ambient_set(matRef, value); }
        public float Specular { get => MatDll.dll_specular_get(matRef); set => MatDll.dll_specular_set(matRef, value); }
        public float Shininess { get => MatDll.dll_shininess_get(matRef); set => MatDll.dll_shininess_set(matRef, value); }

        public bool IsDeleted => Ref.IsValid(matRef);


        public DynamicMaterial(IMaterial source) {
            Console.WriteLine($"#: new DynamicMaterial(source:{source.matRef})");

            matRef = dll_CreateDynamicMaterial(Game.meshAssetRef, source.matRef);

            byte[] buf = new byte[80];
            MatDll.dll_name_get(matRef, buf, buf.Length);

            Name = Encoding.ASCII.GetString(buf);
        }

        public void Delete() {
            dll_DeleteDynamicMaterial(Game.meshAssetRef, matRef);
        }


        [DllImport(MonoClass.ExePath, EntryPoint = "MeshAsset" + "_" + "CreateDynamicMaterial")]
        private static extern CppRef dll_CreateDynamicMaterial(CppRef meshAssetRef, CppRef otherMaterialRef);

        [DllImport(MonoClass.ExePath, EntryPoint = "MeshAsset" + "_" + "DeleteDynamicMaterial")]
        private static extern void dll_DeleteDynamicMaterial(CppRef meshAssetRef, CppRef otherMaterialRef);

    }

    class MatDll {

        [DllImport(MonoClass.ExePath, EntryPoint = "Material" + "_" + "name_get")]
        public static extern int dll_name_get(CppRef compRef, byte[]buf, int bufLength);


        [DllImport(MonoClass.ExePath, EntryPoint = "Material"+"_"+"diffuseColor"+"_get")]
        public static extern Vector3 dll_diffuseColor_get(CppRef compRef);

        [DllImport(MonoClass.ExePath, EntryPoint = "Material" + "_" + "diffuse_get")]
        public static extern float dll_diffuse_get(CppRef compRef);

        [DllImport(MonoClass.ExePath, EntryPoint = "Material" + "_" + "ambient_get")]
        public static extern float dll_ambient_get(CppRef compRef);

        [DllImport(MonoClass.ExePath, EntryPoint = "Material" + "_" + "specular_get")]
        public static extern float dll_specular_get(CppRef compRef);

        [DllImport(MonoClass.ExePath, EntryPoint = "Material" + "_" + "shininess_get")]
        public static extern float dll_shininess_get(CppRef compRef);


        [DllImport(MonoClass.ExePath, EntryPoint = "Material" + "_" + "diffuseColor_set")]
        public static extern void dll_diffuseColor_set(CppRef compRef, Vector3 value);

        [DllImport(MonoClass.ExePath, EntryPoint = "Material" + "_" + "diffuse_set")]
        public static extern void dll_diffuse_set(CppRef compRef, float value);

        [DllImport(MonoClass.ExePath, EntryPoint = "Material" + "_" + "ambient_set")]
        public static extern void dll_ambient_set(CppRef compRef, float value);

        [DllImport(MonoClass.ExePath, EntryPoint = "Material" + "_" + "specular_set")]
        public static extern void dll_specular_set(CppRef compRef, float value);

        [DllImport(MonoClass.ExePath, EntryPoint = "Material" + "_" + "shininess_set")]
        public static extern void dll_shininess_set(CppRef compRef, float value);
    }
}
