using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

using EngineMono;

namespace Engine {
    sealed class CameraComponent : CppComponent {
        private Prop<float> prop_orthoWidth = new Prop<float>(0);
        private Prop<float> prop_orthoHeight = new Prop<float>(1);
        private Prop<float> prop_orthoNearPlane = new Prop<float>(2);
        private Prop<float> prop_orthoFarPlane = new Prop<float>(3);

        private Prop<float> prop_nearPlane = new Prop<float>(4);
        private Prop<float> prop_farPlane = new Prop<float>(5);

        private Prop<bool> prop_drawDebug = new Prop<bool>(6);


        public float OrthoWidth { get => prop_orthoWidth.value; set => prop_orthoWidth.value = value; }
        public float OrthoHeight { get => prop_orthoHeight.value; set => prop_orthoHeight.value = value; }
        public float OrthoNearPlane { get => prop_orthoNearPlane.value; set => prop_orthoNearPlane.value = value; }
        public float OrthoFarPlane { get => prop_orthoFarPlane.value; set => prop_orthoFarPlane.value = value; }

        public float NearPlane { get => prop_nearPlane.value; set => prop_nearPlane.value = value; }
        public float FarPlane { get => prop_farPlane.value; set => prop_farPlane.value = value; }

        public bool DrawDebug { get => prop_drawDebug.value; set => prop_drawDebug.value = value; }

        public bool IsAttached => dll_IsAttached_get(cppRef);
        public bool Orthographic { get => dll_orthographic_get(cppRef); set => dll_orthographic_set(cppRef, value); }

        public void Attach() => dll_Attach(cppRef);
        public void UpdateProjMatrix() => dll_UpdateProjMatrix(cppRef);


        public override CppObjectInfo CreateFromCS(GameObject target) {
            return dll_Create(target.cppRef, csRef);
        }


        [DllImport(MonoClass.ExePath, EntryPoint = "CameraComponent_Create")]
        private static extern CppObjectInfo dll_Create(CppRef cppObjRef, CsRef csCompRef);

        [DllImport(MonoClass.ExePath, EntryPoint = "CameraComponent_IsAttached_get")]
        private static extern bool dll_IsAttached_get(CppRef cppObjRef);

        [DllImport(MonoClass.ExePath, EntryPoint = "CameraComponent_orthographic_get")]
        private static extern bool dll_orthographic_get(CppRef cppObjRef);

        [DllImport(MonoClass.ExePath, EntryPoint = "CameraComponent_orthographic_set")]
        private static extern void dll_orthographic_set(CppRef cppObjRef, bool value);

        [DllImport(MonoClass.ExePath, EntryPoint = "CameraComponent_Attach")]
        private static extern void dll_Attach(CppRef cppObjRef);

        [DllImport(MonoClass.ExePath, EntryPoint = "CameraComponent_UpdateProjMatrix")]
        private static extern void dll_UpdateProjMatrix(CppRef cppObjRef);
    }
}
