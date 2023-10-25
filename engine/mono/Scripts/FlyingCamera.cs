using System;

using Engine;


[GUID("d4806d9b-d18b-4936-873c-a356a8f50288")]
public class FlyingCamera : CSComponent {

    [Open] private CameraComponent m_camera;

    public float rotationSense = 0.2f;

    private bool m_updateRotation = false;
    private Vector3 m_rotationDelta;
    private float m_speed = 100.0f;

    public override void OnInit() {
        if (m_camera == null)
            throw new Exception($"{GetType().FullName}: Not all fields are valid");

        Input.OnMouseMove += m_OnMouseMove;
        Input.OnWheelMove += m_OnWheelMove;
    }

    public override void OnDestroy() {
        Input.OnMouseMove -= m_OnMouseMove;
        Input.OnWheelMove -= m_OnWheelMove;
    }

    public override void OnStart() {
        m_camera.UpdateProjMatrix();
        // m_camera.Attach();
    }

    public override void OnFixedUpdate() {
        if (!m_camera.IsAttached)
            return;

        if (m_updateRotation) {
            var lrot = actor.localRotation;
            lrot += m_rotationDelta;
            actor.localRotation = lrot;
            m_rotationDelta = Vector3.Zero;
            m_updateRotation = false;
        }

        var axis = Vector3.Zero;
        if (Input.GetButton(Key.W)) axis += Vector3.Forward;
        if (Input.GetButton(Key.S)) axis += Vector3.Backward;
        if (Input.GetButton(Key.A)) axis += Vector3.Left;
        if (Input.GetButton(Key.D)) axis += Vector3.Right;
        if (Input.GetButton(Key.Space)) axis += Vector3.Up;
        if (Input.GetButton(Key.LeftShift)) axis += Vector3.Down;
        axis = axis.Normalized();

        var rot = actor.localRotation;
        var rotator = Quaternion.CreateFromYawPitchRoll(rot.Y, rot.X, 0);
        var rotatorForward = Vector3.Forward.Rotate(rotator);
        var rotatorRight = Vector3.Right.Rotate(rotator);
        var rotatorUp = Vector3.Up.Rotate(rotator);

        var direction = rotatorForward * -axis.Z + Vector3.Up * axis.Y + rotatorRight * axis.X;
        direction = direction.Normalized();

        var newPos = actor.localPosition + direction * m_speed * Game.DeltaFixedTime;
        actor.localPosition = newPos;

        var newMatrix = Matrix4x4.CreateLookAt(actor.worldPosition, actor.worldPosition + rotatorForward, rotatorUp);
        m_camera.viewMatrix = newMatrix;
    }

    private void m_OnMouseMove() {
        if (!m_camera.IsAttached)
            return;

        m_rotationDelta.Y -= Input.MouseDelta.X * rotationSense;
        m_rotationDelta.X -= Input.MouseDelta.Y * rotationSense;
        m_updateRotation = true;
    }

    public void m_OnWheelMove() {
        if (!m_camera.IsAttached)
            return;

        if (Input.WheelDelta > 0)
            m_speed += 1;
        if (Input.WheelDelta < 0)
            m_speed -= 1;
    }

}