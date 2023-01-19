using System;
using Engine;

[GUID("d8510879-625e-4254-81f1-4b0f253a0098")]
class MeleeEnemy : CSComponent, IEnemy
{
    private int m_health = 100;

    public void Fight()
    {

    }

    public void Shoot()
    {

    }

    public void Death()
    {
        if (m_health == 0)
            this.Destroy();
    }

    public void Damage(int damage)
    {
        if (m_health > 0)
            m_health -= damage;
        else
            m_health = 0;
    }

    public override void OnInit()
    {
    }

    public override void OnUpdate()
    {
    }
}