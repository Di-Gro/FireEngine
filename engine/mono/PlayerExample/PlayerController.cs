using System;

using Engine;

#if BADCODE

public class PlayerController : CSComponent {

    Player m_player = null;

    public override void OnStart() {
        m_player = actor.GetComponent<Player>();
        if (m_player == null)
            Console.WriteLine("PlayerController: Player component don't found.");
    }

    public override void OnUpdate() {
        if (m_player == null)
            return;

        var axis = Vector3.Zero;
        if (Input.GetButtonDown(Key.W)) axis += Vector3.Right;
        if (Input.GetButtonDown(Key.S)) axis += Vector3.Left;
        if (Input.GetButtonDown(Key.A)) axis += Vector3.Forward;
        if (Input.GetButtonDown(Key.D)) axis += Vector3.Backward;
        axis = axis.Normalize();

        if (Input.GetButtonDown(Key.LeftShift)) 
            axis *= 2;

        if (axis != Vector3.Zero)
            m_player.Move(axis);
    }

}

#endif