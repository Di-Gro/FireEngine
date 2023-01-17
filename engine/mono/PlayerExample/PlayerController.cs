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

    public override void OnUpdate() {

        var inputAxis = Vector3.Zero;
        if (Input.GetButton(Key.W)) inputAxis += Vector3.Forward;
        if (Input.GetButton(Key.S)) inputAxis += Vector3.Backward;
        if (Input.GetButton(Key.A)) inputAxis += Vector3.Left;
        if (Input.GetButton(Key.D)) inputAxis += Vector3.Right;
        inputAxis = inputAxis.Normalized();
        
        if (Input.GetButton(Key.LeftShift))
            inputAxis *= 2;
            
        Move(inputAxis);
    }

    public void Move(Vector3 axis) {
       
        var cameraForward = m_playerCamera.actor.forward;
        var cameraRight = m_playerCamera.actor.right;

        cameraForward = (cameraForward * new Vector3(1, 0, 1)).Normalized();

        var direction = cameraForward * axis.Z + cameraRight * -axis.X;

        var vel = direction * speed * Game.DeltaTime;

        var velocity = m_character.GetLinearVelocity();
        velocity.X = vel.X;
        velocity.Z = vel.Z;
        // velocity.X = (vel.X / 4 + velocity.X) / 2;
        // velocity.Z = (vel.Z / 4 + velocity.Z) / 2;
        m_character.SetLinearVelocityClamped(velocity);

        // m_character.AddForce(vel);
        // m_character.AddImpulse(vel);
    }
}