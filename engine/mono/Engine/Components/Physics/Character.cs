using EngineDll;

namespace Engine {

    [GUID("481ac014-eb4a-4a25-8ef8-c40db6efbb02")]
    public class Character : CppComponent {

        public bool active {
            get => Dll.Character.active_get(cppRef);
            set => Dll.Character.active_set(cppRef, value);
        }

        public float MaxSlopeAngle { get => prop_MaxSlopeAngle.value; set => prop_MaxSlopeAngle.value = value; }
        public float Mass { get => prop_mass.value; set => prop_mass.value = value; }

        public float gravity {
            get => Dll.Character.gravity_get(cppRef);
            set => Dll.Character.gravity_set(cppRef, value);
        }

        public float Friction { get => prop_Friction.value; set => prop_Friction.value = value; }

        public Vector3 GetLinearVelocity() => Dll.Character.GetLinearVelocity(cppRef);
        public void SetLinearVelocityClamped(Vector3 inLinearVelocity) => Dll.Character.SetLinearVelocityClamped(cppRef, inLinearVelocity);

        public void AddImpulse(Vector3 inImpulse) => Dll.Character.AddImpulse(cppRef, inImpulse);

        public float GetFriction() => Dll.Character.GetFriction(cppRef);
        public void SetFriction(float inFriction) => Dll.Character.SetFriction(cppRef, inFriction);


        public override CppObjectInfo CppConstructor() => Dll.Character.Create(csRef);

        private Prop<float> prop_MaxSlopeAngle = new Prop<float>(0);
        private Prop<float> prop_Friction = new Prop<float>(1);
        private Prop<float> prop_mass = new Prop<float>(2);
    }
}