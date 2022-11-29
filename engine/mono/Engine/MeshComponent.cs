using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

using EngineMono;


namespace Engine {

    sealed class MeshComponent : CppComponent {
        private Prop<bool> prop_IsDebug = new Prop<bool>(0);
        private Prop<bool> prop_Visible = new Prop<bool>(1);
         
        public bool IsDebug { get => prop_IsDebug.value; set => prop_IsDebug.value = value; }
        public bool IsVisible { get => prop_Visible.value; set => prop_Visible.value = value; }
        public bool IsStatic => dll_IsStatic(cppRef);
        public bool IsDynamic => dll_IsDynamic(cppRef);
        public int MaterialCount => dll_MaterialCount_get(cppRef);

        public Mesh mesh {
            get => m_mesh;
            set {
                m_mesh = value;
                dll_SetFromCs(cppRef, mesh.meshRef);
            }
        }

        private Mesh m_mesh;


        public void AddShape(Vertex[] verteces, int[] indeces, int materialIndex) {
            dll_AddShape(cppRef, verteces, verteces.Length, indeces, indeces.Length, 0);
        }

        public void SetMaterial(ulong index, IMaterial material) {
            dll_SetMaterial(cppRef, index, material.matRef);
        }

        public StaticMaterial GetMaterial(ulong index) {
            Console.WriteLine($"#:  MeshComponent.GetMaterial({index})");

            CppRef matRef = dll_GetMaterial(cppRef, index);
            Console.WriteLine($"#:  MeshComponent.dll_GetMaterial({cppRef}, {index}) -> {matRef}");

            return new StaticMaterial(matRef);
        }

        public void RemoveMaterials() {
            dll_RemoveMaterials(cppRef);
        }

        public void RemoveMaterial(int index) {
            dll_RemoveMaterial(cppRef, index);
        }

        public void ClearMesh() {
            dll_ClearMesh(cppRef);
        }

        public override CppObjectInfo CreateFromCS(GameObject target) {
            return dll_Create(target.cppRef, csRef);
        }


        private static void cpp_SetFromCpp(CsRef compRef, CppRef meshRef) {
            var component = GetObjectByRef(compRef) as MeshComponent;

            if (meshRef.value == 0)
                component.m_mesh = null;
            else
                component.m_mesh = new Mesh(meshRef);
        }


        [DllImport(MonoClass.ExePath, EntryPoint = "MeshComponent" + "_" + "Create")]
        private static extern CppObjectInfo dll_Create(CppRef cppGameObjRef, CsRef csCompRef);


        [DllImport(MonoClass.ExePath, EntryPoint = "MeshComponent" + "_" + "IsDynamic_get")]
        private static extern bool dll_IsDynamic(CppRef cppGameObjRef);

        [DllImport(MonoClass.ExePath, EntryPoint = "MeshComponent" + "_" + "IsStatic_get")]
        private static extern bool dll_IsStatic(CppRef cppGameObjRef);

        [DllImport(MonoClass.ExePath, EntryPoint = "MeshComponent" + "_" + "MaterialCount_get")]
        private static extern int dll_MaterialCount_get(CppRef cppGameObjRef);


        [DllImport(MonoClass.ExePath, EntryPoint = "MeshComponent" + "_" + "SetFromCs")]
        private static extern void dll_SetFromCs(CppRef compRef, CppRef meshRef);


        [DllImport(MonoClass.ExePath, EntryPoint = "MeshComponent" + "_" + "AddShape", CallingConvention = CallingConvention.Cdecl)]
        private static extern void dll_AddShape(CppRef compRef, Vertex[] verteces, int vlength, int[] indeces, int ilength, int matIndex);


        [DllImport(MonoClass.ExePath, EntryPoint = "MeshComponent" + "_" + "RemoveMaterials")]
        private static extern void dll_RemoveMaterials(CppRef compRef);

        [DllImport(MonoClass.ExePath, EntryPoint = "MeshComponent" + "_" + "RemoveMaterial")]
        private static extern void dll_RemoveMaterial(CppRef compRef, int index);

        [DllImport(MonoClass.ExePath, EntryPoint = "MeshComponent" + "_" + "ClearMesh")]
        private static extern void dll_ClearMesh(CppRef compRef);


        [DllImport(MonoClass.ExePath, EntryPoint = "MeshComponent" + "_" + "SetMaterial")]
        private static extern void dll_SetMaterial(CppRef compRef, ulong index, CppRef materialRef);

        [DllImport(MonoClass.ExePath, EntryPoint = "MeshComponent" + "_" + "GetMaterial")]
        private static extern CppRef dll_GetMaterial(CppRef compRef, ulong index);
    }
}
