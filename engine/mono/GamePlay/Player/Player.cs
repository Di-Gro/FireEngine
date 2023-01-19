using System;

using Engine;

[GUID("416a2b38-e34a-44fd-aa1e-429d30c44ba2")]
public class Player : CSComponent, IPlayer {
    public Prefab bulletPrefab;
    public PlayerCamera camera;

    public float bulletSpawnOffset = 30;
    public float bulletSpawnHeight = 50;

    private int m_health = 100;

    private bool m_isPickuped = false;
    private bool m_isShoot = false;

    [Open] Actor m_meshLHand = null;
    [Open] Actor m_meshRHand = null;
    [Open] Actor m_pickupedSlot = null;

    private Vector3 m_handsPosition;

    public override void OnInit()
    {
        if (m_meshLHand == null || m_meshRHand == null)
            throw new NullFieldException(this);

        m_handsPosition = m_meshLHand.localRotation;
    }

    public void PickupObject()
    {
        if (!m_isPickuped)
        {
            m_isPickuped = true;
            if(m_pickupedSlot.GetChildrenCount() > 0)
                m_pickupedSlot.GetChild(0).localPosition = m_pickupedSlot.localPosition;
            Console.WriteLine("Pickuped Object!");
        }
    }

    public void DropObject()
    {
        m_isPickuped = false;
        if(m_pickupedSlot.GetChildrenCount() > 0)
        {
            m_pickupedSlot.GetChild(0).AddComponent<BoxCollider>();
            m_pickupedSlot.GetChild(0).AddComponent<Rigidbody>().AddForce(new Vector3(10, 0, 0));
            m_pickupedSlot.GetChild(0).AddComponent<Rigidbody>().simulate = true;
            //m_pickupedSlot.GetChild(0).parent = null;
            ThowObjectAnimation();
            Console.WriteLine("Object dropped!");
        }
    }

    public void ThowObjectAnimation()
    {
        //m_pickupedSlot.GetChild(0).localRotation.Lerp((m_pickupedSlot.GetChild(0).localRotation.X, (m_pickupedSlot.GetChild(0).localRotation.Y + 90.0f), m_pickupedSlot.GetChild(0).localRotation.Z), 10);
        m_pickupedSlot.GetChild(0).worldRotationQ.SetY(m_pickupedSlot.GetChild(0).worldRotationQ.Y + 90.0f);
    }

    public void Shoot()
    {
        if(m_isPickuped && m_isShoot) {
            m_isPickuped = false;
            m_isShoot = false;
        }

        bool testShoot = true;
        if(testShoot) {
            if (bulletPrefab != null && camera != null) {
                var bullet = bulletPrefab.Instanciate();

                var cameraForward = camera.CameraForward();

                var wpos = actor.worldPosition;
                wpos += cameraForward * bulletSpawnOffset;
                wpos.Y = bulletSpawnHeight;

                bullet.worldPosition = wpos;
                
                //TODO: To be continued
            }
        }
    }

    public void Death()
    {
        if (m_health == 0)
            this.Destroy();
    }

    public void Damage(int damage)
    {
        if(m_health > 0)
            m_health -= damage;
        else
            m_health = 0;
    }

    public override void OnUpdate()
    {
        if (Input.GetButton(Key.E))
            PickupObject();

        if(Input.GetButton(Key.Q))
            DropObject();

        if (Input.GetButton(Key.R)) {
            if (!m_isShoot)
                m_isShoot = true;
            else
                m_isShoot = false;
        }
        if (Input.GetButtonDown(Key.LeftButton)) {
            Shoot();
        }
       
    }
}