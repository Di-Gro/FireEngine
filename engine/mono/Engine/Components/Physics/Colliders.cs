using EngineDll;

namespace Engine {

    [GUID("73d3cbc8-cb32-4e71-a8bb-570b6e71ac45")]

#if DETACHED

    /// <summary> Detached ready </summary>
    class BoxCollider : CppComponent {

        public Vector3 Size { get; set; }
        public bool DrawDebug { get; set; }

        public override CppObjectInfo CppConstructor() {
            throw new System.NotImplementedException();
        }
    }

#else

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

#endif

    [GUID("aa06e9b3-f75f-4982-976b-a6f57568d6f0")]
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

    [GUID("6124c3d2-97b5-4b8f-882e-7be50df9016b")]
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