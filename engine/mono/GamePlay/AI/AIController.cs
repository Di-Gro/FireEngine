using System;

using Engine;

[GUID("1a233fe3-9cdc-4658-a3af-290a66f55a69")]
public class AIController : CSComponent {
    [Open] private Player m_player;


    public override void OnInit() {
        if (m_player == null)
            throw new NullFieldException(this);
    }

    public override void OnUpdate() {
        /// Здесь можно раздать команды
        
        // if (Input.GetButtonDown(Key.LeftButton)) {
        //     if (m_player.CanThrow)
        //         m_player.Throw();

        //     else if (m_player.CanDrop)
        //         m_player.Drop();

        //     else if (m_player.CanPickup)
        //         m_player.Pickup();
        // }
    }

    public override void OnFixedUpdate() {
        /// Здесь можно установить вектор движения
        
        // var direction = Vector3.Forward;

        // bool jump = false;
        // bool run = false;

        // m_player.SetMovementVector(direction, jump, run, Game.DeltaFixedTime);
    }
}