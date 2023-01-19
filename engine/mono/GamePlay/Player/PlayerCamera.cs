using System;
using System.Collections.Generic;

using Engine;

[GUID("a49b6d24-fe71-4965-a421-36645951c2c3")]
public class PlayerCamera : CSComponent {

    [Open] private CameraComponent m_camera;
    [Open] private Actor m_cameraTarget;

    public float rotationSense = 1.0f;
    public float scrollSense = 1.0f;
    public float distance = 100;

    [Open] private Vector3 m_angles = Vector3.Zero;
    

    public override void OnInit() {
        if (m_cameraTarget == null || m_camera == null)
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
        m_camera.Attach();
    }

    public override void OnUpdate() { }

    public override void OnFixedUpdate() {
        if (!m_camera.IsAttached)
            return;


        var rot = Quaternion.CreateFromAxisAngle(Vector3.Up, -m_angles.X) 
                * Quaternion.CreateFromAxisAngle(Vector3.Right, m_angles.Y);

        var cameraBackward = Vector3.Forward.Rotate(rot);
        var cameraPos = m_cameraTarget.worldPosition + cameraBackward * distance;

        actor.worldPosition = cameraPos;
        actor.localRotationQ = -rot;
        
        var newMatrix = Matrix4x4.CreateLookAt(cameraPos, m_cameraTarget.worldPosition, Vector3.Up);
        m_camera.viewMatrix = newMatrix;
    }

    private void m_OnMouseMove() {
        if (!m_camera.IsAttached)
            return;

        m_angles.X += Input.MouseDelta.X * rotationSense;
        m_angles.Y += Input.MouseDelta.Y * rotationSense;
        m_angles.Y = Math.Clamp(m_angles.Y, 20, 80);
    }

    public void m_OnWheelMove() {
        if (!m_camera.IsAttached)
            return;

        float dir = Input.WheelDelta > 0 ? -1 : 1;
        distance += dir * 2 * scrollSense;
        distance = Math.Clamp(distance, 1, 10000);
    }
}