
using Engine;

[GUID("d1a15b62-0f3a-4466-a4ac-a98d0640e35e")]
public class TileMap : CSComponent {

    [Open] private Prefab m_playerPrefab;
    [Space]
    [Open] private Actor m_playerStart;

    private Player m_player;


    public override void OnInit() {
        if(m_playerPrefab == null)
            throw new NullFieldException(this);
    }

    public override void OnStart() {
        m_player = m_playerPrefab.Instanciate().GetComponent<Player>();
        m_player.actor.worldPosition = m_playerStart.worldPosition;
        
    }

}