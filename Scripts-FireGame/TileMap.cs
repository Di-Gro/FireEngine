using System;
using Engine;

[GUID("d1a15b62-0f3a-4466-a4ac-a98d0640e35e")]
public class Spawner : CSComponent {

    [Open] private Prefab m_playerPrefab;
    [Open] private Prefab m_enemyPrefab;
    [Open] private Prefab m_itemPrefab;
    [Space]
    [Open] private Actor m_playerStart;

    public int sizeX = 3;
    public int sizeY = 3;

    public float tileSize = 800;

    private Player m_player;

    private Random m_random = new Random();


    public override void OnInit() {
        if(m_playerPrefab == null || m_enemyPrefab == null || m_itemPrefab == null || m_playerStart == null)
            throw new NullFieldException(this);
    }

    public override void OnStart() {
        m_player = m_playerPrefab.Instanciate().GetComponent<Player>();
        m_player.actor.worldPosition = m_playerStart.worldPosition;

        m_Spawn();
    }

    private void m_Spawn() {
        for (int ix = 0; ix < sizeX; ix++) {
        
            for (int iy = 0; iy < sizeY; iy++) {
                var center = new Vector3(ix, 0, iy) * tileSize + (Vector3.One * (tileSize / 2));

                int itemCount = m_random.Next(1, 4);
                for (int i = 0; i < itemCount; i++)
                    m_SpawnItem(center);
            }
        }
    }

    private void m_SpawnItem(Vector3 center) {
        var x = tileSize / 2 - m_random.Next(0, (int)tileSize + 1);
        var y = tileSize / 2 - m_random.Next(0, (int)tileSize + 1);
        var pos = new Vector3(x, 15, y);

        var item = m_itemPrefab.Instanciate();

        item.localScale = Vector3.One * 20;
        item.worldPosition = pos;
    }

    private void m_SpawnEnemy(Vector3 center) {
        var x = tileSize / 2 - m_random.Next(0, (int)tileSize + 1);
        var y = tileSize / 2 - m_random.Next(0, (int)tileSize + 1);
        var pos = new Vector3(x, 15, y);

        var enemy = m_enemyPrefab.Instanciate();
        var player = enemy.GetComponent<Player>();

        enemy.worldPosition = pos;
    }
}