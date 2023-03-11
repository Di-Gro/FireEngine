using System;

interface IPlayer
{
    public void Pickup();
    public void Throw();
    public void Drop();
    public void Death();
    public void AddDamage(int damage);
}
