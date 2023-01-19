using System;

using Engine;

[GUID("416a2b38-e34a-44fd-aa1e-429d30c44ba2")]
public class Player : CSComponent, IPlayer {
    private int m_health = 100;

    private bool m_isPickuped = false;
    private bool m_isShoot = false;

    [Open] Actor m_meshLHand = null;
    [Open] Actor m_meshRHand = null;
    [Open] Actor m_pickupedSlot = null;

    private Vector3 m_handsPosition;

    public override void OnInit()
    {
        if (m_meshLHand == null || m_meshRHand == null)
            throw new NullFieldException(this);

        m_handsPosition = m_meshLHand.localRotation;
    }

    public void PickupObject()
    {
        if (!m_isPickuped)
        {
            m_isPickuped = true;
            if(m_pickupedSlot.GetChildrenCount() > 0)
                m_pickupedSlot.GetChild(0).localPosition = m_pickupedSlot.localPosition;
            Console.WriteLine("Pickuped Object!");
        }
    }

    public void DropObject()
    {
        m_isPickuped = false;
        if(m_pickupedSlot.GetChildrenCount() > 0)
        {
            m_pickupedSlot.GetChild(0).AddComponent<BoxCollider>();
            m_pickupedSlot.GetChild(0).AddComponent<Rigidbody>().AddForce(new Vector3(10, 0, 0));
            m_pickupedSlot.GetChild(0).AddComponent<Rigidbody>().active = true;
            //m_pickupedSlot.GetChild(0).parent = null;
            ThowObjectAnimation();
            Console.WriteLine("Object dropped!");
        }
    }

    public void ThowObjectAnimation()
    {
        //m_pickupedSlot.GetChild(0).localRotation.Lerp((m_pickupedSlot.GetChild(0).localRotation.X, (m_pickupedSlot.GetChild(0).localRotation.Y + 90.0f), m_pickupedSlot.GetChild(0).localRotation.Z), 10);
        m_pickupedSlot.GetChild(0).worldRotationQ.SetY(m_pickupedSlot.GetChild(0).worldRotationQ.Y + 90.0f);
    }

    public void Shoot()
    {
        if(m_isPickuped && m_isShoot)
        {
            m_isPickuped = false;
            m_isShoot = false;
        }
    }

    public void Death()
    {
        if (m_health == 0)
            this.Destroy();
    }

    public void Damage(int damage)
    {
        if(m_health > 0)
            m_health -= damage;
        else
            m_health = 0;
    }

    public override void OnUpdate()
    {
        if (Input.GetButton(Key.E))
            PickupObject();
        if(Input.GetButton(Key.Q))
            DropObject();
        if (Input.GetButton(Key.R))
        {
            if (!m_isShoot)
                m_isShoot = true;
            else
                m_isShoot = false;
        }
    }
}

#if BADCODE

public class Player : Engine.CSComponent {
    public float radius = 25;
    public float speed = 100;

    private PlayerCamera m_playerCamera = null;
    private Actor m_playerMesh = null;
    private Actor m_playerBound = null;

    private float m_rotationSpeed = 0;
    private float m_startRadius = 0;
    private float m_startSpeed = 0;

    private Vector3 m_velocityDelta;
    private Vector3 m_lastVelocity;
    private Quaternion m_rotationDelta;

    private bool m_updatePosition = false;
    private bool m_updateRotation = false;

    private MeshComponent m_boxMesh;
    private MeshComponent m_boundSphere;

    private DynamicMaterial m_boxMeshMaterial;
    private DynamicMaterial m_boundSphereMaterial;

    public override void OnInit() {
        m_rotationSpeed = rad(speed / (2 * pi * radius) * 360);
        m_startRadius = radius;
        m_startSpeed = speed;

        m_playerMesh = new Actor("player mesh", actor);
        m_playerBound = new Actor("player bound", actor);

        m_boxMeshMaterial = new DynamicMaterial(Assets.ShaderDiffuseColor);
        m_boxMeshMaterial.DiffuseColor = new Vector3(0.8f, 0.6f, 0.2f);

        m_boundSphereMaterial = new DynamicMaterial(Assets.ShaderDiffuseColor);
        m_boundSphereMaterial.DiffuseColor = new Vector3(0.0f, 0.6f, 0.0f);

        float boxSize = radius * 2 * 0.9f;
        m_boxMesh = m_playerMesh.AddComponent<MeshComponent>();
        m_boxMesh.actor.localScale = new Vector3(boxSize, boxSize, boxSize);
        m_boxMesh.mesh = new StaticMesh().LoadFromFile(MeshAsset.formBox);
        m_boxMesh.SetMaterial(0, m_boxMeshMaterial);

        m_boundSphere = m_playerBound.AddComponent<MeshComponent>();
        m_boundSphere.actor.localScale = new Vector3(radius * 2, radius * 2, radius * 2);
        m_boundSphere.mesh = new StaticMesh().LoadFromFile(MeshAsset.formSphereLined);
        m_boundSphere.SetMaterial(0, m_boundSphereMaterial);
        m_boundSphere.IsDebug = false;
        m_boundSphere.CastShadow = false;
	}

    public override void OnDestroy() {
        m_boxMeshMaterial.Delete();
        m_boundSphereMaterial.Delete();
    }

    public override void OnStart() {
        m_playerCamera = actor.GetComponentInChild<PlayerCamera>();
        if (m_playerCamera == null)
            Console.WriteLine("Player haven't got a PlayerCamera component");

        var pos = actor.localPosition;
        pos.Y = radius;
        actor.localPosition = pos;
    }

    public override void OnUpdate() {
        if (m_updatePosition) {
            actor.localPosition = actor.localPosition + m_velocityDelta;
            m_lastVelocity = m_velocityDelta;
            m_velocityDelta = Vector3.Zero;
            m_updatePosition = false;
        }
        if (m_updateRotation) {
            var rot = m_playerMesh.localRotationQ;
            rot *= m_rotationDelta;

            m_playerMesh.localRotationQ = rot;
            m_playerBound.localRotationQ = rot;

            m_rotationDelta = new Quaternion();
            m_updateRotation = false;
        }

        var point = actor.worldPosition;

        if (m_lastVelocity != Vector3.Zero)
            point += m_lastVelocity.Normalize() * radius + m_lastVelocity;
	}

    public void Move(Vector3 axis) {
        var forward = m_playerCamera.actor.forward;
        forward.Y = 0;
        forward.Normalize();

        var direction = forward * axis.X + m_playerCamera.actor.right * axis.Z;

        if (direction != Vector3.Zero) {
            var cross = direction.Cross(Vector3.Up);
            float angleDelta = m_rotationSpeed * direction.Length() * Game.DeltaTime;

            m_velocityDelta += direction * speed * Game.DeltaTime;
            m_rotationDelta *= Quaternion.CreateFromAxisAngle(-cross, angleDelta);

            m_updatePosition = true;
            m_updateRotation = true;
        }
	}

    public void Attach(Actor attachableObj) {
        attachableObj.parent = m_playerMesh;

        var attachable = attachableObj.GetComponentInChild<Attachable>();

        radius += attachable.boundRadius * 0.02f;
        float prog = radius / m_startRadius;
        speed = m_startSpeed * prog;

        var scale = m_playerBound.localScale;
        scale = Vector3.One * m_startRadius * 2 * prog;
        m_playerBound.localScale = scale;

        var pos = actor.localPosition;
        pos.Y = radius;
        actor.localPosition = pos;
    }
};

#endif
