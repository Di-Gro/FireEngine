using System;

using Engine;

[GUID("1cd3c575-0b05-4a62-b0dd-3b326c3f7608")]
public class PlayerController : CSComponent {

    [Open] private PlayerCamera m_playerCamera;
    [Open] private Player m_player;


    public override void OnInit() {
        if (m_playerCamera == null || m_player == null)
            throw new NullFieldException(this);
    }

    public override void OnUpdate() {
    
        if (Input.GetButtonDown(Key.LeftButton)) {
            if (m_player.CanThrow)
                m_player.Throw();

            else if (m_player.CanDrop)
                m_player.Drop();

            else if (m_player.CanPickup)
                m_player.Pickup();
        }
    }

    public override void OnFixedUpdate() {

        var direction = Vector3.Zero;
        if (Input.GetButton(Key.W)) direction += Vector3.Forward;
        if (Input.GetButton(Key.S)) direction += Vector3.Backward;
        if (Input.GetButton(Key.A)) direction += Vector3.Left;
        if (Input.GetButton(Key.D)) direction += Vector3.Right;
        direction = direction.Normalized();

        direction = m_ToCameraRelativeDiretcion(direction);

        bool jump = Input.GetButtonDown(Key.Space);
        bool run = Input.GetButton(Key.LeftShift);
       
        m_player.SetMovementVector(direction, jump, run, Game.DeltaFixedTime);
    }

    private Vector3 m_ToCameraRelativeDiretcion(Vector3 axis) {
        var cameraForward = m_playerCamera.CameraForward();
        var cameraRight = m_playerCamera.actor.right;

        return cameraForward * axis.Z + cameraRight * -axis.X;
    }
}