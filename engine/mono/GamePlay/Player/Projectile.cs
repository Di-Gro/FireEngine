using System;

namespace Engine {

    [GUID("c6f30bd4-5f87-4417-9682-51d56f404a02")]
    public class Prijectile : CSComponent {

        public float speed = 300;
        public float distance = 500;

        private Rigidbody m_rigidbody;

        private Vector3 m_direction;
        private Vector3 m_startPos;

        private bool m_onFly = false;

        public override void OnInit() {
            m_rigidbody = actor.GetComponent<Rigidbody>();
            if(m_rigidbody == null)
                throw new NullFieldException(this);
        }

        public void Shoot(Vector3 direction) {
            m_direction = direction;
            m_startPos = actor.worldPosition;
        }

        public override void OnFixedUpdate() {
            if (!m_onFly)
                return;

            var velosity = m_direction * speed;
            var nextPos = actor.worldPosition + velosity * Game.DeltaFixedTime;
            var dist = (nextPos - m_startPos).Length();

            Console.WriteLine($"{dist}");

            m_rigidbody.SetLinearVelocityClamped(velosity);
        }

    }
}