using System;

interface IPlayer
{
    public void PickupObject();
    public void DropObject();
    public void Shoot();
    public void Death();
    public void Damage(int damage);
}
