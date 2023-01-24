using System;

using Engine;
using System.Timers;

[GUID("1a233fe3-9cdc-4658-a3af-290a66f55a69")]
public class AIController : CSComponent {
    [Open] private Player m_player;
    [Open] private Player m_target;
    [Open] private AIComponent m_ai;
    private float can_shoot_dt;
    private Random rnd = new Random();
    private Vector3 next_point;
    private Vector3 old_target_pos;
    int index_in_path = 0;
    private static bool build_navmesh = false;
    Vector3[] path = new Vector3[0];

    private static int count_ai = 0;
    int id = 0;
    public bool in_senses_radius_h => (m_player.CharacterPosititon.Distance(m_target.CharacterPosititon)) <= 400;
    public bool in_senses_radius_l => (m_player.CharacterPosititon.Distance(m_target.CharacterPosititon)) > 200;
    public bool in_senses_radius => in_senses_radius_l && in_senses_radius_h;

 
    public bool out_range => (m_player.CharacterPosititon.Distance(m_target.CharacterPosititon)) > 400;
    public bool in_attack_range => m_player.CharacterPosititon.Distance(m_target.CharacterPosititon) < 200;
    public bool can_shoot => can_shoot_dt > 2 && in_attack_range;
    public float distance_to_player => (m_player.CharacterPosititon.Distance(m_target.CharacterPosititon));
    public override void OnInit() {
        m_ai = actor.AddComponent<AIComponent>();
        if (m_player == null || m_ai == null)
            throw new NullFieldException(this);
    }

    public override void OnStart() {
        next_point = m_player.CharacterPosititon;
        m_target = SceneData.player;
        id = count_ai;
        count_ai++;
        
        old_target_pos = m_target.CharacterPosititon;
        var tag_can_shoot = nameof(can_shoot);
        var tag_in_senses_radius = nameof(in_senses_radius);
        var tag_out_range = nameof(out_range);
        m_ai.Add(this, tag_in_senses_radius);
        m_ai.Add(this, tag_can_shoot);
        m_ai.AddDecision("Random roam", random_roam);
        m_ai.AddDecision("Move to player", move_to_player);
        m_ai.AddDecision("OnAttack", attack);
        m_ai["Random roam"].Add(this, tag_out_range, "True");
        m_ai["Move to player"].Add(this, tag_in_senses_radius, "True");
        m_ai["OnAttack"].Add(this, tag_can_shoot, "True");
    }

    public override void OnUpdate() {
        /// Здесь можно раздать команды
        if (m_target!= null && m_target.IsDestroyed)
        {
            m_target = null;
            
        }
        if(m_target == null)
        {
            return;
        }
        m_ai.DecideAll();
        can_shoot_dt += Game.DeltaTime;

    }

    public override void OnFixedUpdate() {
        /// Здесь можно установить вектор движения
        ///
        if(m_target == null)
        {
            return;
        }
        // Console.WriteLine($"TargetPos {next_point}");
        // Console.WriteLine($"MyPos {m_player.CharacterPosititon}");
        next_point.Y = m_player.CharacterPosititon.Y;
        if (next_point.Distance(m_player.CharacterPosititon) > 15)
        {
            var direction = (next_point - m_player.CharacterPosititon).Normalized();
            bool jump = false;
            bool run = false;
            m_player.SetMovementVector(direction, jump, run, Game.DeltaFixedTime);

        }
        m_player.ViewDirection = (m_target.CharacterPosititon - m_player.CharacterPosititon).Normalized();
    }

    void move_to_player()
    {
        // Console.WriteLine("move_to_player");
        if (old_target_pos.Distance(m_target.CharacterPosititon) > 10 || path.Length == 0)
        {
            
            var scale_direction = (m_target.CharacterPosititon - m_player.CharacterPosititon).Normalized() * 200;
            scale_direction = scale_direction.RotateY(22-rnd.Next(1, 46));
            var end_point = m_player.CharacterPosititon + scale_direction;
            int Vertexes = NavMesh.FindPath(m_player.CharacterPosititon, end_point, id, 0);
            path = NavMesh.GetPath(id);

            old_target_pos = m_target.CharacterPosititon;
            index_in_path = 0;
            next_point = path[index_in_path];

        }
        if (next_point.Distance(m_player.CharacterPosititon) <= 10)
        {
            if(index_in_path + 1 < path.Length)
            {
                index_in_path++;
                next_point = path[index_in_path];
            }
        }
    }


    void random_roam()
    {
        // Console.WriteLine("random_roam");
        if(next_point == m_player.CharacterPosititon)
            next_point = NavMesh.RandomPoint();

    }


    void attack()
    {

        can_shoot_dt = 0 - (float)rnd.NextDouble() * 3;
        m_player.Shoot();
        // Console.WriteLine("Attack");
    }
}
