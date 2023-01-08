using EngineDll;

namespace Engine {
    public class AmbientLight : CppComponent {

        private Prop<Vector3> prop_Color = new Prop<Vector3>(0);
        private Prop<float> prop_Intensity = new Prop<float>(1);

        public Vector3 Color { get => prop_Color.value; set => prop_Color.value = value; }
        public float Intensity { get => prop_Intensity.value; set => prop_Intensity.value = value; }

        public override CppObjectInfo CppConstructor() {
            return Dll.AmbientLight.Create(csRef);
        }
    }
}