using EngineDll;

namespace Engine {

    [GUID("f5bf5659-901d-40b8-9950-28c468110098", typeof(PointLight))]
    public class PointLight : CppComponent {

        private Prop<Vector3> prop_Color = new Prop<Vector3>(0);
        private Prop<float> prop_Intensity = new Prop<float>(1);
        private Prop<float> prop_Radius = new Prop<float>(2);

        [Range(0f, 1f)] [Color] public Vector3 Color { get => prop_Color.value; set => prop_Color.value = value; }
        [Range(0f, 1f)] public float Intensity { get => prop_Intensity.value; set => prop_Intensity.value = value; }
        public float Radius { get => prop_Radius.value; set => prop_Radius.value = value; }

        public override CppObjectInfo CppConstructor() {
            return Dll.PointLight.Create(csRef);
        }
    }
}