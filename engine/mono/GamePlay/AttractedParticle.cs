using System;
using Engine;


[GUID("1dba1ea0-9cb1-4c1d-bdf2-1acac058bcef")]
public class AttractedParticle : CSComponent {

    public ItemType type = ItemType.Trash;

    public float throwSpeed = 300;
    public float throwDistance = 150;
    [Space]
    public float dropSpeed = 50;
    public float dropDistance = 0;
    [Space]
    public bool canThrow = true;
    public bool canPickup = true;
    [Space]
    [Open] private Actor m_trigger;

    private Projectile m_projectile;

    public bool InHands { get; private set; } = false;


    public override void OnInit() {
        m_projectile = actor.GetComponent<Projectile>();

        if (m_projectile == null)
            throw new NullFieldException(this, "m_projectile == null");

        if (m_trigger == null)
            throw new NullFieldException(this, "m_trigger == null");

        actor.Flags |= Flag.IsItem;

        m_trigger.ActiveSelf = false;
    }

    public override void OnStart() {
        m_projectile.SleepEvent += m_OnProjectileSleep;
        m_projectile.AwakeEvent += m_OnProjectileAwake;

        Simulate(true);
    }

    public void Simulate(bool value) {
        InHands = false;

        m_ShootProjectile(Vector3.Zero, 0, 0);
    }

    public void Pickup() {
        InHands = true;

        m_projectile.Enabled = false;
        m_trigger.ActiveSelf = false;
    }

    public void Drop(Vector3 direction) {
        InHands = false;

        m_ShootProjectile(direction, dropSpeed, dropDistance);
    }

    public void Throw(Vector3 direction) {
        InHands = false;

        m_ShootProjectile(direction, throwSpeed, throwDistance);
    }

    private void m_ShootProjectile(Vector3 direction, float speed, float distance) {
        bool useGravity = speed == 0;

        m_projectile.speed = speed;
        m_projectile.distance = distance;
        m_projectile.Shoot(direction, useGravity);
    }

    private int tmp_count = 0;

    private void m_OnProjectileSleep() {
        Console.WriteLine($"Projectile Sleep: {tmp_count++}");

        if (canPickup && !m_trigger.ActiveSelf)
            m_trigger.ActiveSelf = true;
    }

    private void m_OnProjectileAwake() {
        Console.WriteLine($"Projectile Awake: {tmp_count++}");

        if (m_trigger.ActiveSelf)
            m_trigger.ActiveSelf = false;
    }

}