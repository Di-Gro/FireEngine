using EngineDll;

namespace Engine {

    [GUID("d6a2321e-553b-40ba-af46-00c043d446ca", typeof(SpotLight))]
    public class SpotLight : CppComponent {

        private Prop<Vector3> prop_Color = new Prop<Vector3>(0);
        private Prop<float> prop_Intensity = new Prop<float>(1);
        private Prop<float> prop_Length = new Prop<float>(2);
        private Prop<float> prop_Angle = new Prop<float>(3);
        private Prop<float> prop_Blend = new Prop<float>(4);
        private Prop<float> prop_Attenuation = new Prop<float>(5);

        [Range(0f, 1f)] [Color] public Vector3 Color { get => prop_Color.value; set => prop_Color.value = value; }
        [Range(0f, 1f)] public float Intensity { get => prop_Intensity.value; set => prop_Intensity.value = value; }
        public float Length { get => prop_Length.value; set => prop_Length.value = value; }
        [Range(0f, 179.999f)] public float Angle { get => prop_Angle.value; set => prop_Angle.value = value; }
        [Range(0f, 1f)] public float Blend { get => prop_Blend.value; set => prop_Blend.value = value; }
        public float Attenuation { get => prop_Attenuation.value; set => prop_Attenuation.value = value; }

        public override CppObjectInfo CppConstructor() {
            return Dll.SpotLight.Create(csRef);
        }
    }
}