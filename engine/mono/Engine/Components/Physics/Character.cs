using System;

using EngineDll;

namespace Engine {

    [GUID("481ac014-eb4a-4a25-8ef8-c40db6efbb02")]
    public class Character : CppComponent {

        public bool simulate {
            get => Dll.Character.simulate_get(cppRef);
            set => Dll.Character.simulate_set(cppRef, value);
        }

        public float MaxSlopeAngle { get => prop_MaxSlopeAngle.value; set => prop_MaxSlopeAngle.value = value; }
        public float Mass { get => prop_mass.value; set => prop_mass.value = value; }

        public float gravity {
            get => Dll.Character.gravity_get(cppRef);
            set => Dll.Character.gravity_set(cppRef, value);
        }

        public float WalkSpeed { get => prop_walkSpeed.value; set => prop_walkSpeed.value = value; }
        public float RunSpeed { get => prop_runSpeed.value; set => prop_runSpeed.value = value; }
        public float JumpSpeed { get => prop_jumpSpeed.value; set => prop_jumpSpeed.value = value; }
        [Close] public Vector3 Velosity { get => prop_velosity.value; }

        public float Friction { get => prop_Friction.value; set => prop_Friction.value = value; }

        public Vector3 GetLinearVelocity() => Dll.Character.GetLinearVelocity(cppRef);
        public void SetLinearVelocityClamped(Vector3 inLinearVelocity) => Dll.Character.SetLinearVelocityClamped(cppRef, inLinearVelocity);

        public void AddImpulse(Vector3 inImpulse) => Dll.Character.AddImpulse(cppRef, inImpulse);

        public float GetFriction() => Dll.Character.GetFriction(cppRef);
        public void SetFriction(float inFriction) => Dll.Character.SetFriction(cppRef, inFriction);

        public void HandleInput(Vector3 movementDirection, bool jump, bool run, float deltaTime) {
            Dll.Character.HandleInput(cppRef, movementDirection, jump, run, deltaTime);
        }


        public override CppObjectInfo CppConstructor() => Dll.Character.Create(csRef);

        private Prop<float> prop_MaxSlopeAngle = new Prop<float>(0);
        private Prop<float> prop_Friction = new Prop<float>(1);
        private Prop<float> prop_mass = new Prop<float>(2);
        private Prop<float> prop_walkSpeed = new Prop<float>(3);
        private Prop<float> prop_jumpSpeed = new Prop<float>(4);
        private Prop<Vector3> prop_velosity = new Prop<Vector3>(5);
        private Prop<float> prop_runSpeed = new Prop<float>(6);

        // public override void OnTriggerEnter(Actor otherActor, in Contact contact) {
        //     Console.WriteLine($"OnTriggerEnter: {actor.Name} -> {otherActor.Name}");
        // }

        // public override void OnTriggerExit(Actor otherActor) {
        //     Console.WriteLine($"OnTriggerExit: {actor.Name} -> {otherActor.Name}");
        // }

        // public override void OnCollisionEnter(Actor otherActor, in Contact contact) {
        //     Console.WriteLine($"OnCollisionEnter: {actor.Name} -> {otherActor.Name}");
        // }

        // public override void OnCollisionExit(Actor otherActor) {
        //     Console.WriteLine($"OnCollisionExit: {actor.Name} -> {otherActor.Name}");
        // }
    }
}