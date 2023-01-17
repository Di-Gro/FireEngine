using System;

namespace Engine.AI {
    
    [GUID("ec0f9944-49f0-49ae-a94f-b34c5cc72acc")]
    public class AITestComponent : CSComponent {

        public int ammo = 5;
        public bool IsLowAmmo => ammo < 3;

        private AIComponent m_ai;


        public override void OnInit() {
            m_ai = actor.AddComponent<AIComponent>();

            var tag_ammo = nameof(ammo);
            var tag_isLowAmmo = nameof(IsLowAmmo);

            m_ai.Add(this, tag_ammo);
            m_ai.Add(this, tag_isLowAmmo);
            m_ai.Add("other", tag_isLowAmmo, "True");
            
            m_ai.AddDecision("reload", OnReload);
            m_ai["reload"].Add(this, tag_isLowAmmo, "True");

            m_ai.AddDecision("any reload");
            m_ai["any reload"].Add("#x", tag_isLowAmmo, "True");
        }

        public override void OnUpdate() {
            m_ai.DecideAll();

            if(m_ai["any reload"].IsDecided)
                Console.WriteLine("any reload: IsDecided");
        }

        void OnReload() {
            Console.WriteLine("OnReload");
        }

    }
}