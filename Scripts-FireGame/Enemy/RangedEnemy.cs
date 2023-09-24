using System;
using Engine;

[GUID("05ae01ca-deba-4e80-b689-5193ef24bf0d")]
class RangedEnemy : CSComponent, IEnemy
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
