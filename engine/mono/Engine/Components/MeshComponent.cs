using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

using EngineDll;
using FireYaml;


namespace Engine {

    [GUID("acec5527-e867-41b9-a852-ecfbf26e4491", typeof(MeshComponent))]
    sealed class MeshComponent : CppComponent {

        private Prop<bool> prop_IsDebug = new Prop<bool>(0);
        private Prop<bool> prop_Visible = new Prop<bool>(1);
        private Prop<Vector3> prop_MeshScale = new Prop<Vector3>(2);
         
        public Vector3 MeshScale { get => prop_MeshScale.value; set => prop_MeshScale.value = value; }
        public bool IsDebug { get => prop_IsDebug.value; set => prop_IsDebug.value = value; }
        public bool CastShadow {
            get => Dll.MeshComponent.castShadow_get(cppRef);
            set => Dll.MeshComponent.castShadow_set(cppRef, value);
        }
        public bool IsVisible { get => prop_Visible.value; set => prop_Visible.value = value; }
        public bool IsStatic => Dll.MeshComponent.IsStatic_get(cppRef);
        public bool IsDynamic => Dll.MeshComponent.IsDynamic_get(cppRef);
        public int MaterialCount => Dll.MeshComponent.MaterialCount_get(cppRef);

        [Close] public Mesh mesh {
            get => m_mesh;
            set {
                m_mesh = value;
                Dll.MeshComponent.SetFromCs(cppRef, mesh.cppRef);
            }
        }

        private Mesh m_mesh;


        public void AddShape(Vertex[] verteces, int[] indeces, int materialIndex) {
            Dll.MeshComponent.AddShape(cppRef, verteces, verteces.Length, indeces, indeces.Length, 0);
        }

        public void SetMaterial(ulong index, IMaterial material) {
            Dll.MeshComponent.SetMaterial(cppRef, index, material.cppRef);
        }

        public StaticMaterial GetMaterial(ulong index) {
            //Console.WriteLine($"#:  MeshComponent.GetMaterial({index})");

            CppRef matRef = Dll.MeshComponent.GetMaterial(cppRef, index);
            //Console.WriteLine($"#:  MeshComponent.dll_GetMaterial({cppRef}, {index}) -> {matRef}");

            return new StaticMaterial(matRef);
        }

        public void RemoveMaterials() {
            Dll.MeshComponent.RemoveMaterials(cppRef);
        }

        public void RemoveMaterial(int index) {
            Dll.MeshComponent.RemoveMaterial(cppRef, index);
        }

        public void ClearMesh() {
            Dll.MeshComponent.ClearMesh(cppRef);
        }

        public override CppObjectInfo CppConstructor(/*Actor target*/) {
            return Dll.MeshComponent.Create(/*target.cppRef,*/ csRef);
        }


        private static void cpp_SetFromCpp(CsRef compRef, CppRef meshRef) {
            var component = GetObjectByRef(compRef) as MeshComponent;

            if (meshRef.value == 0) {
                component.m_mesh = null;
                return;
            }
            if (component.IsStatic) {
                var assetHash = Dll.Mesh4.assetIdHash_get(meshRef);
                component.m_mesh = new StaticMesh(assetHash);
            }
            else {
                component.m_mesh = new Mesh(meshRef);
            }
        }
    }
}
