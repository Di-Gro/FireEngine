using System;

using Engine;

[GUID("1cd3c575-0b05-4a62-b0dd-3b326c3f7608")]
public class PlayerController : CSComponent {

    [Open] private Player m_player;
    [Open]  private PlayerCamera m_playerCamera;

    public float speed = 1;

    private Character m_character;

    public override void OnInit() {
        if (m_player == null || m_playerCamera == null)
            throw new NullFieldException(this);

        m_character = m_player.actor.GetComponent<Character>();
        if (m_character == null)
            throw new NullFieldException(this);
    }

    public override void OnFixedUpdate() {

        var direction = Vector3.Zero;
        if (Input.GetButton(Key.W)) direction += Vector3.Forward;
        if (Input.GetButton(Key.S)) direction += Vector3.Backward;
        if (Input.GetButton(Key.A)) direction += Vector3.Left;
        if (Input.GetButton(Key.D)) direction += Vector3.Right;
        direction = direction.Normalized();

        bool jump = Input.GetButtonDown(Key.Space);
        bool run = Input.GetButton(Key.LeftShift);

        direction = ToCameraRelativeDiretcion(direction);

        m_character.HandleInput(direction, jump, run, Game.DeltaFixedTime);
    }

    public Vector3 ToCameraRelativeDiretcion(Vector3 axis) {

        var cameraForward = m_playerCamera.CameraForward();
        var cameraRight = m_playerCamera.actor.right;

        return cameraForward * axis.Z + cameraRight * -axis.X;

        // var cameraForward = m_playerCamera.actor.forward;
        // cameraForward = (cameraForward * new Vector3(1, 0, 1)).Normalized();
    }
}