using System;

interface IEnemy
{
    public void Fight();
    public void Shoot();
    public void Death();
    public void Damage(int damage);
}