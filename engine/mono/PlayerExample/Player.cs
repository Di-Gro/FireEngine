using System;
using System.Collections.Generic;

using Engine; 

[GUID("416a2b38-e34a-44fd-aa1e-429d30c44ba2")]
public class Player : CSComponent {
    public Prefab bulletPrefab;
    public PlayerCamera camera;

    public float bulletSpawnOffset = 30;
    public float bulletSpawnHeight = 50;

    public override void OnInit() { }

    public override void OnUpdate() {
        if (bulletPrefab != null && camera != null) {
            if (Input.GetButtonDown(Key.LeftButton)) {
                var bullet = bulletPrefab.Instanciate();

                var cameraForward = camera.CameraForward();

                var wpos = actor.worldPosition;
                wpos += cameraForward * bulletSpawnOffset;
                wpos.Y = bulletSpawnHeight;

                bullet.worldPosition = wpos;
            }
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