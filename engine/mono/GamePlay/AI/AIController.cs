using System;

using Engine;

[GUID("1a233fe3-9cdc-4658-a3af-290a66f55a69")]
public class AIController : CSComponent {
    [Open] private Player m_player;
    [Open] private Player m_target;
    [Open] private AIComponent m_ai;

    public bool in_senses_radius_h => (m_player.CharacterPosititon.Distance(m_target.CharacterPosititon)) <= 1000;
    public bool in_senses_radius_l => (m_player.CharacterPosititon.Distance(m_target.CharacterPosititon)) > 500;
    public bool in_senses_radius => in_senses_radius_l && in_senses_radius_h;
    public bool in_attack_range => m_player.CharacterPosititon.Distance(m_target.CharacterPosititon) < 500;

    public float distance_to_player; 
    public override void OnInit() {
        m_ai = actor.AddComponent<AIComponent>();
        if (m_player == null || m_target == null || m_ai == null)
            throw new NullFieldException(this);
        var tag_in_attack_range = nameof(in_attack_range);
        var tag_in_senses_radius = nameof(in_senses_radius);
        m_ai.Add(this, tag_in_senses_radius);
        m_ai.Add(this, tag_in_attack_range);
        m_ai.AddDecision("Random roam", random_roam);
        m_ai.AddDecision("Move to player", move_to_player);
        m_ai.AddDecision("OnAttack", attack);
        m_ai["Random roam"].Add(this, tag_in_senses_radius, "False");
        m_ai["Move to player"].Add(this, tag_in_senses_radius, "True");
        m_ai["OnAttack"].Add(this, tag_in_attack_range, "True");

    }

    public override void OnUpdate() {
        /// Здесь можно раздать команды
        distance_to_player = m_player.CharacterPosititon.Distance(m_target.CharacterPosititon);
        Console.WriteLine($"Direction { m_target.CharacterPosititon - m_player.CharacterPosititon}");
        Console.WriteLine($"Distance {distance_to_player}");
        m_ai.DecideAll();
    }

    public override void OnFixedUpdate() {
        /// Здесь можно установить вектор движения
        // var direction = Vector3.Forward;

        // bool jump = false;
        // bool run = false;

        // m_player.SetMovementVector(direction, jump, run, Game.DeltaFixedTime);
    }

    void move_to_player()
    {
        Console.WriteLine("move_to_player");
    }


    void random_roam()
    {
        Console.WriteLine("random_roam");
    }


    void attack()
    {
        Console.WriteLine("Attack");
    }
}