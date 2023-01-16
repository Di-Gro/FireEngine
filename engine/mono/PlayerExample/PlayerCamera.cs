using System;
using System.Collections.Generic;

using Engine;

#if BADCODE

public class PlayerCamera : CameraComponent {

    private float m_rotationSense = 1.0f;
    private float m_scrollSense = 1.0f;

    private bool m_updateRotation = false;
    private bool m_updateDistance = false;

    private Vector3 m_rotationDelta;
    private float m_distanceDelta;

    private Actor m_cameraRoot = null;

    public override void OnInit() {
        m_cameraRoot = actor.parent;
        if (m_cameraRoot == null)
            Console.WriteLine("PlayerCamera haven't got a parent\n");

        Input.OnMouseMove += m_OnMouseMove;
        Input.OnWheelMove += m_OnWheelMove;

        UpdateProjMatrix();
        Attach();
    }

    public override void OnDestroy() {
        Input.OnMouseMove -= m_OnMouseMove;
        Input.OnWheelMove -= m_OnWheelMove;
    }

    public override void OnUpdate() {
        if (!IsAttached)
            return;

        if (m_updateRotation) {
            var rot = m_cameraRoot.localRotation;
            rot += m_rotationDelta;
            m_cameraRoot.localRotation = rot;
            m_rotationDelta = Vector3.Zero;
            m_updateRotation = false;
        }
        if (m_updateDistance) {
            var pos = actor.localPosition;
            pos.Z += m_distanceDelta;
            actor.localPosition = pos;
            m_distanceDelta = 0;
            m_updateDistance = false;
        }
        {
            var rot = m_cameraRoot.localRotation;
            var rotator = Matrix4x4.CreateFromYawPitchRoll(rot.Y, rot.X, 0);

            var wpos = actor.worldPosition;
            var newMatrix = Matrix4x4.CreateLookAt(wpos, wpos + rotator.Forward, rotator.Up);
            viewMatrix = newMatrix;
            UpdateProjMatrix();
        }
    }

    private void m_OnMouseMove() {
        if (!IsAttached)
            return;

        m_rotationDelta.Y -= Input.MouseDelta.X * 0.003f * m_rotationSense;
        m_rotationDelta.X -= Input.MouseDelta.Y * 0.003f * m_rotationSense;
        m_updateRotation = true;
    }

    public void m_OnWheelMove() {
        if (!IsAttached)
            return;

        float dir = Input.WheelDelta > 0 ? -1 : 1;
        m_distanceDelta += dir * 2 * m_scrollSense;
        m_updateDistance = true;
    }
};

#endif