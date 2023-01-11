using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

using EngineDll;

namespace Engine {

    public sealed class CameraComponent : CppComponent {
        private Prop<float> prop_orthoWidth = new Prop<float>(0);
        private Prop<float> prop_orthoHeight = new Prop<float>(1);
        private Prop<float> prop_orthoNearPlane = new Prop<float>(2);
        private Prop<float> prop_orthoFarPlane = new Prop<float>(3);

        private Prop<float> prop_nearPlane = new Prop<float>(4);
        private Prop<float> prop_farPlane = new Prop<float>(5);

        private Prop<bool> prop_drawDebug = new Prop<bool>(6);
        private Prop<bool> prop_isPlayerCamera = new Prop<bool>(7);

        public bool IsPlayerCamera { get => prop_isPlayerCamera.value; set => prop_isPlayerCamera.value = value; }

        public float OrthoWidth { get => prop_orthoWidth.value; set => prop_orthoWidth.value = value; }
        public float OrthoHeight { get => prop_orthoHeight.value; set => prop_orthoHeight.value = value; }
        public float OrthoNearPlane { get => prop_orthoNearPlane.value; set => prop_orthoNearPlane.value = value; }
        public float OrthoFarPlane { get => prop_orthoFarPlane.value; set => prop_orthoFarPlane.value = value; }

        public float NearPlane { get => prop_nearPlane.value; set => prop_nearPlane.value = value; }
        public float FarPlane { get => prop_farPlane.value; set => prop_farPlane.value = value; }

        public bool DrawDebug { get => prop_drawDebug.value; set => prop_drawDebug.value = value; }
        
        public bool IsAttached => Dll.CameraComponent.IsAttached_get(cppRef);

        public bool Orthographic { 
            get => Dll.CameraComponent.orthographic_get(cppRef); 
            set => Dll.CameraComponent.orthographic_set(cppRef, value); 
        }

        [Close] 
        public Matrix4x4 viewMatrix {
            get => Dll.CameraComponent.viewMatrix_get(cppRef);
            set => Dll.CameraComponent.viewMatrix_set(cppRef, value);
        }

        public void Attach() => Dll.CameraComponent.Attach(cppRef);
        public void UpdateProjMatrix() => Dll.CameraComponent.UpdateProjMatrix(cppRef);


        public override CppObjectInfo CppConstructor() {
            return Dll.CameraComponent.Create(csRef);
        }

    }
}
