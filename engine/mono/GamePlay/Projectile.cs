using System;

namespace Engine {

    [GUID("c6f30bd4-5f87-4417-9682-51d56f404a02")]
    public class Projectile : CSComponent {

        public enum DestroyRule { NonDestroy, DestroyOnCollide, DestroyOnSleep, DestroyAfterTime }

        public Action DestroyEvent;
        public Action SleepEvent;
        public Action AwakeEvent;
        public Action CollideEvent;

        public float speed = 300;
        public float distance = 300;

        public DestroyRule destroy = DestroyRule.NonDestroy;

        public float sleepVelosity = 1;
        public float notSleepTime = 1;
        public bool canAwake = true;

        public bool IsSleep { get; private set; } = true;

        [Close] public bool OnFly { get => m_onFly; }

        [Close] public bool Enabled { 
            get => m_enabled; 
            set {
                m_enabled = value;
                m_onFly = false;
                m_rigidbody.simulate = m_enabled;
            } 
        }

        private bool m_enabled = false;

        private Rigidbody m_rigidbody;

        private Vector3 m_direction;
        private Vector3 m_startPos;
        private float m_startGravity;

        float m_timeFromShoot = 0;

        private bool m_onFly = false;
        
        private bool hasCollide = false;

        public override void OnInit() {
            m_rigidbody = actor.GetComponent<Rigidbody>();
            if(m_rigidbody == null)
                throw new NullFieldException(this, "Rigidbody not found");
        }

        public void Shoot(Vector3 direction, bool useGravity = false) {
            m_direction = direction;
            m_startPos = actor.worldPosition;
            m_startGravity = m_rigidbody.gravity;

            m_rigidbody.simulate = true;
            if(!useGravity)
                m_rigidbody.gravity = 0;

            var velosity = m_direction * speed;
            m_rigidbody.SetLinearVelocityClamped(velosity);

            m_rigidbody.AddForce(Vector3.One * 1, Vector3.Down * actor.localScale.Y);

            Enabled = true;
            m_onFly = true;
            IsSleep = false;

            m_timeFromShoot = 0;
        }

        public override void OnUpdate() {
            if (!Enabled)
                return;

            m_timeFromShoot += Game.DeltaTime;

            if (m_HandleSleep())
                SleepEvent?.Invoke();

            if (m_HandleAwake())
                AwakeEvent?.Invoke();

            if (m_HandleDestroy()) {
                DestroyEvent?.Invoke();
                return;
            }
        }

        public override void OnDestroy() {
            DestroyEvent = null;
            SleepEvent = null;
            CollideEvent = null;
        }

        public override void OnFixedUpdate() {
            if (!Enabled)
                return;

            if (m_onFly) {
                var dist = (actor.worldPosition - m_startPos).Length();

                if (dist >= distance && m_rigidbody.gravity == 0) {
                    m_rigidbody.gravity = m_startGravity;
                    m_onFly = false;
                }
            }
        }

        public override void OnCollisionEnter(Actor otherActor, in Contact contact) {
            if (!Enabled)
                return;

            if (otherActor.Has(Flag.Player))
                return;

            m_rigidbody.gravity = m_startGravity;
            m_onFly = false;
            hasCollide = true;

            CollideEvent?.Invoke();
        }

        private bool m_HandleSleep() {
            if (!IsSleep) {
                if (m_timeFromShoot >= notSleepTime) {
                    var velosity = m_rigidbody.GetLinearVelocity();
                    if (velosity.Length() <= sleepVelosity) {
                        IsSleep = true;
                        return true;
                    }
                }
            }
            return false;
        }

        private bool m_HandleAwake() {
            if(IsSleep && canAwake) {
                var velosity = m_rigidbody.GetLinearVelocity();
                if (velosity.Length() > sleepVelosity)
                    IsSleep = false;

                return !IsSleep;
            }
            return false;
        }


        private bool m_HandleDestroy() {
            if (hasCollide && destroy == DestroyRule.DestroyOnCollide) {
                actor.Destroy();
                return true;
            }
            if (m_timeFromShoot >= notSleepTime && destroy == DestroyRule.DestroyAfterTime) {
                actor.Destroy();
                return true;
            }
            if (IsSleep && destroy == DestroyRule.DestroyOnSleep) {
                actor.Destroy();
                return true;
            }
            return false;
        }

        
    }
}