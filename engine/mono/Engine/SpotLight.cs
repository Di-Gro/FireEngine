using EngineDll;

namespace Engine {
    public class SpotLight : CppComponent {

        private Prop<Vector3> prop_Color = new Prop<Vector3>(0);
        private Prop<float> prop_Intensity = new Prop<float>(1);
        private Prop<float> prop_Length = new Prop<float>(2);
        private Prop<float> prop_Angle = new Prop<float>(3);
        private Prop<float> prop_Blend = new Prop<float>(4);
        private Prop<float> prop_Attenuation = new Prop<float>(5);

        public Vector3 Color { get => prop_Color.value; set => prop_Color.value = value; }
        public float Intensity { get => prop_Intensity.value; set => prop_Intensity.value = value; }
        public float Length { get => prop_Length.value; set => prop_Length.value = value; }
        public float Angle { get => prop_Angle.value; set => prop_Angle.value = value; }
        public float Blend { get => prop_Blend.value; set => prop_Blend.value = value; }
        public float Attenuation { get => prop_Attenuation.value; set => prop_Attenuation.value = value; }

        public override CppObjectInfo CppConstructor() {
            return Dll.SpotLight.Create(csRef);
        }
    }
}