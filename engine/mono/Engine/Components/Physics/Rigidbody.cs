using System;

using EngineDll;

namespace Engine {

    public enum MotionType { Static, Kinematic, Dynamic }
    public enum MotionQuality { Discrete, LinearCast }


    [GUID("6338b2f1-49da-4a86-8c6b-a2a975e644c4")]
    public class Rigidbody : CppComponent {

        [Close] public ComponentCallbacks.ContactEnter TriggerEnterEvent;
        [Close] public ComponentCallbacks.ContactExit TriggerExitEvent;

        public bool simulate {
            get => Dll.Rigidbody.simulate_get(cppRef);
            set => Dll.Rigidbody.simulate_set(cppRef, value);
        }

        public bool isSensor { get => prop_isSensor.value; set => prop_isSensor.value = value; }

        public MotionType motion{
            get => (MotionType) Dll.Rigidbody.motion_get(cppRef);
            set => Dll.Rigidbody.motion_set(cppRef, (uint)value); 
        }

        public float Mass { get => prop_mass.value; set => prop_mass.value = value; }

        public float gravity{
            get => Dll.Rigidbody.gravity_get(cppRef);
            set => Dll.Rigidbody.gravity_set(cppRef, value);
        }
       
        public float MaxLinearVelocity { get => prop_maxLinearVelocity.value; set => prop_maxLinearVelocity.value = value; }
        public float LinearDamping { get => prop_linearDamping.value; set => prop_linearDamping.value = value; }
        public float AgularDamping { get => prop_angularDamping.value; set => prop_angularDamping.value = value; }
        public float Friction { get => prop_Friction.value; set => prop_Friction.value = value; }
        public float Bounciness { get => prop_Bounciness.value; set => prop_Bounciness.value = value; }
        public bool AllowSleeping { get => prop_AllowSleeping.value; set => prop_AllowSleeping.value = value; }
        public bool KeepLocal { get => prop_keepLocal.value; set => prop_keepLocal.value = value; }

        public MotionQuality Quality {
            get => (MotionQuality)Dll.Rigidbody.quality_get(cppRef);
            set => Dll.Rigidbody.quality_set(cppRef, (uint)value);
        }

        public void AddForce(Vector3 inForce) => Dll.Rigidbody.AddForce(cppRef, inForce);
        public void AddForce(Vector3 inForce, Vector3 inPosition) => Dll.Rigidbody.AddForceInPos(cppRef, inForce, inPosition);

        public Vector3 GetLinearVelocity() => Dll.Rigidbody.GetLinearVelocity(cppRef);
        public void SetLinearVelocityClamped(Vector3 inLinearVelocity) => Dll.Rigidbody.SetLinearVelocityClamped(cppRef, inLinearVelocity);
        
        public void AddImpulse(Vector3 inImpulse) => Dll.Rigidbody.AddImpulse(cppRef, inImpulse);
        public void AddImpulse(Vector3 inImpulse, Vector3 inPosition) => Dll.Rigidbody.AddImpulseInPos(cppRef, inImpulse, inPosition);
      
        public float GetFriction() => Dll.Rigidbody.GetFriction(cppRef);
        public void SetFriction(float inFriction) => Dll.Rigidbody.SetFriction(cppRef, inFriction);


        public override CppObjectInfo CppConstructor() => Dll.Rigidbody.Create(csRef);

        private Prop<float> prop_mass = new Prop<float>(0);
        private Prop<float> prop_maxLinearVelocity = new Prop<float>(1);
        private Prop<float> prop_linearDamping = new Prop<float>(2);
        private Prop<float> prop_angularDamping = new Prop<float>(3);
        private Prop<float> prop_Friction = new Prop<float>(4);
        private Prop<float> prop_Bounciness = new Prop<float>(5);
        private Prop<bool> prop_AllowSleeping = new Prop<bool>(6);
        private Prop<bool> prop_isSensor = new Prop<bool>(7);
        private Prop<bool> prop_keepLocal = new Prop<bool>(8);

        public override void OnTriggerEnter(Actor otherActor, in Contact contact) {
            // Console.WriteLine($"OnTriggerEnter: {actor.Name} -> {otherActor.Name}");
            TriggerEnterEvent?.Invoke(otherActor, in contact);
        }

        public override void OnTriggerExit(Actor otherActor) {
            TriggerExitEvent?.Invoke(otherActor);
        }


        // public override void OnCollisionEnter(Actor otherActor, in Contact contact) {
        //     Console.WriteLine($"OnCollisionEnter: {actor.Name} -> {otherActor.Name}");
        // }

        // public override void OnCollisionExit(Actor otherActor) {
        //     Console.WriteLine($"OnCollisionExit: {actor.Name} -> {otherActor.Name}");
        // }
    }
}