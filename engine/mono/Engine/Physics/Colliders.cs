using EngineDll;

namespace Engine {

    class BoxCollider : CppComponent {

        // public Vector3 Center { get => prop_Center.value; set => prop_Center.value = value; }
        public Vector3 Size { get => prop_Size.value; set => prop_Size.value = value; }
        public bool DrawDebug { get => prop_DrawDebug.value; set => prop_DrawDebug.value = value; }


        // private Prop<Vector3> prop_Center = new Prop<Vector3>(0);
        private Prop<Vector3> prop_Size = new Prop<Vector3>(0);
        private Prop<bool> prop_DrawDebug = new Prop<bool>(1);

        public override CppObjectInfo CppConstructor() {
            return Dll.BoxCollider.Create(csRef);
        }
    }

    class SphereCollider : CppComponent {

        // public Vector3 Center { get => prop_Center.value; set => prop_Center.value = value; }
        public float Radius { get => prop_Radius.value; set => prop_Radius.value = value; }
        public bool DrawDebug { get => prop_DrawDebug.value; set => prop_DrawDebug.value = value; }
       

        // private Prop<Vector3> prop_Center = new Prop<Vector3>(0);
        private Prop<float> prop_Radius = new Prop<float>(0);
        private Prop<bool> prop_DrawDebug = new Prop<bool>(1);


        public override CppObjectInfo CppConstructor() {
            return Dll.SphereCollider.Create(csRef);
        }
    };

    class CapsuleCollider : CppComponent {

        // public Vector3 Center { get => prop_Center.value; set => prop_Center.value = value; }
        public float Radius { get => prop_Radius.value; set => prop_Radius.value = value; }
        public float Height { get => prop_Height.value; set => prop_Height.value = value; }
        public bool DrawDebug { get => prop_DrawDebug.value; set => prop_DrawDebug.value = value; }


        // private Prop<Vector3> prop_Center = new Prop<Vector3>(0);
        private Prop<float> prop_Radius = new Prop<float>(0);
        private Prop<float> prop_Height = new Prop<float>(1);
        private Prop<bool> prop_DrawDebug = new Prop<bool>(2);


        public override CppObjectInfo CppConstructor() {
            return Dll.CapsuleCollider.Create(csRef);
        }
    };

}