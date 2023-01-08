using System;
using System.Collections.Generic;

using Engine;

#if BADCODE

namespace PlayerExample {
    public class GameController : Engine.CSComponent {
        private Player m_player;

        public override void OnInit() {
            m_defaultCamera = Game.MainCamera;
            m_defaultCamera.actor.localPosition = new Vector3(0, 100, -300);

            m_player = CreatePlayer().GetComponent<Player>();
            m_playerCamera = m_player.actor.GetComponentInChild<PlayerCamera>();

            //TODO: убрать в сцену
            std::vector<Actor> newObjects;
            game()->meshAsset()->LoadScene("../../data/assets/levels/farm", &newObjects);

            for (auto gobj : newObjects) {
                auto attachable = gobj->GetComponent<Attachable>();
                if (attachable != nullptr)
                    attachable->player = m_player;
            }
        }

        public override void OnUpdate() {
            if (Input.GetButtonDown(Key.Esc)) {
                Console.WriteLine("Exit");
                Game.Exit();
            }
        }

        public Actor CreatePlayer() {
            var player = new Actor("player");
            player.AddComponent<Player>();
            player.AddComponent<PlayerController>();

            var cameraRoot = new Actor("camera root", player);

            var camera = new Actor("player camera", cameraRoot);
            camera.localPosition = new Vector3(0, 0, 300);
            camera.AddComponent<PlayerCamera>();

            return player;
        }

    };
}

#endif