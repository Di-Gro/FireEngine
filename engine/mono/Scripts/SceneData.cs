
using Engine;

[GUID("25772377-922d-4d4a-b0a8-2cd264bff05b")]
public class SceneData : CSComponent {

    public static Actor playerCharacter => m_instance != null ? m_instance.m_playerCharacter : null;
    public static Player player => m_instance != null ? m_instance.m_player : null;
    private static SceneData m_instance = null;

    [Open] private Actor m_playerCharacter;
    [Open] private Player m_player;

    public override void OnInit() => m_instance = this;

}