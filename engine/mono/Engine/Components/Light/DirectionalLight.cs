using Engine;
using EngineDll;

namespace Engine {

    [GUID("a6e37381-5a85-4cd0-b421-77f5f6365495", typeof(DirectionalLight))]
    public class DirectionalLight : CppComponent {

        private Prop<Vector3> prop_Color = new Prop<Vector3>(0);
        private Prop<float> prop_Intensity = new Prop<float>(1);

        [Range(0f, 1f)] [Color] public Vector3 Color { get => prop_Color.value; set => prop_Color.value = value; }
        [Range(0f, 1f)] public float Intensity { get => prop_Intensity.value; set => prop_Intensity.value = value; }

        public override CppObjectInfo CppConstructor() {
            return Dll.DirectionalLight.Create(csRef);
        }
    }
}