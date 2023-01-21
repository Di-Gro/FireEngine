using System;
using System.Collections.Generic;
using Engine;

[GUID("416a2b38-e34a-44fd-aa1e-429d30c44ba2")]
public class Player : CSComponent, IPlayer {

    [Open] private Actor m_body;
    [Open] private Actor m_hands;
    [Open] private Actor m_itemSlot;

    [Space]
    public float shootImpulse = 200;

    [Close] public Item Item { get; protected set; } = null;
    [Close] public Item ItemOnGrpund { get; protected set; } = null;
    [Close] public bool HasItem => Item != null;
    [Close] public bool HasItemOnGround => ItemOnGrpund != null;
    [Close] public bool CanThrow => HasItem && Item.canThrow && !HasDropTarget;
    [Close] public bool CanDrop => HasItem && !CanThrow;
    [Close] public bool CanPickup => !HasItem && HasItemOnGround;
    [Close] public ItemType DropType = ItemType.None;
    [Close] public bool HasDropTarget => HasItem && DropType == Item.type;

    [Close] public Vector3 DropDirection => ViewDirection;

    [Close] public Vector3 ViewDirection { get; set; } = Vector3.Zero;

    [Close] public Character PlayerCharacter => m_character;

    private int m_health = 100;

    private Character m_character;

    private Vector3 m_handsPosition;

    [Open] private Prefab m_itemPrefab;
    [Open] private Prefab m_particlePrefab;

    public float particleDropImpulse = 0.0f;

    private Random rnd = new Random();

    [Close] public Vector3 CharacterPosititon => m_character.actor.worldPosition;

    public override void OnInit() {
        if (m_hands == null || m_itemSlot == null || m_itemPrefab == null || m_particlePrefab == null)
            throw new NullFieldException(this);

        m_character = actor.GetComponentInChild<Character>();
        if (m_character == null)
            throw new NullFieldException(this, "Character not found");

        m_character.TriggerEnterEvent += OnTriggerEnter;
        m_character.TriggerExitEvent += OnTriggerExit;

        m_character.CollisionEnterEvent += OnCollisionEnter;
        m_character.CollisionExitEvent += OnCollisionExit;

        m_handsPosition = m_hands.localRotation;
    }

    public override void OnDestroy() {
        if (!m_character.IsDestroyed) {
            m_character.TriggerEnterEvent -= OnTriggerEnter;
            m_character.TriggerExitEvent -= OnTriggerExit;

            m_character.CollisionEnterEvent -= OnCollisionEnter;
            m_character.CollisionExitEvent -= OnCollisionExit;
        }
    }

    public override void OnUpdate()
    {
        m_RotateBodyToViewDirection();
        if (m_health <= 0)
            Death();
    }
    public void Pickup() {
        if(!HasItemOnGround)
            return;

        Item = ItemOnGrpund;
        ItemOnGrpund = null;

        Item.Pickup();

        Item.actor.parent = m_itemSlot;
        Item.actor.localPosition = Vector3.Zero;
        Item.actor.localRotationQ = Quaternion.Identity;
    }

    public void Throw() {
        var direction = DropDirection;

        Item.actor.parent = null;
        Item.Throw(direction);
        Item = null;

        m_character.AddImpulse(-direction * shootImpulse);
    }

    public void Drop() {
        var direction = DropDirection;

        Item.actor.parent = null;
        Item.Drop(direction);
        Item = null;
    }

    public void DropStaff(Vector3 pos)
    {
        var direction = Vector3.Forward.RotateY(rnd.Next(1, 361));
        var item = m_itemPrefab.Instanciate().GetComponent<Item>();
        
        item.m_simulateOnStart = false;
        item.actor.worldPosition = pos;
        item.Drop(direction);

        //int count = 1;//rnd.Next(2, 3);
        //
        //for (int i = 0; i < count; i++)
        //    m_DropParticle(pos);
    }

    public void Death() {
        m_character.simulate = false;
        DropStaff(m_character.actor.worldPosition);
        actor.Destroy();
    }

    public void AddDamage(int damage) {
        m_health -= damage;
        if (m_health < 0)
            m_health = 0;
        Console.WriteLine($"Health: {m_health}");
    }

    public void AddEnergy(int value) {
        Console.WriteLine($"AddEnergy: {value}");
    }

    public void SetMovementVector(Vector3 direction, bool jump, bool run, float deltaTime) {
        if (direction != Vector3.Zero)
            ViewDirection = direction;

        m_character.HandleInput(direction, jump, run, Game.DeltaFixedTime);
    }

    public override void OnTriggerEnter(Actor otherActor, in Contact contact) {
        m_HandleItemEnter(otherActor);
        m_HandleDropTargetEnter(otherActor);
    }

    public override void OnTriggerExit(Actor otherActor)
    {
        m_HandleItemExit(otherActor);
        m_HandleDropTargetExit(otherActor);
    }

    public override void OnCollisionEnter(Actor otherActor, in Contact contact)
    {
        m_HandleItemDamaged(otherActor);
        Console.WriteLine("OnCollisionEnter");
    }

    private void m_HandleItemEnter(Actor otherActor) {
        if (!otherActor.Has(Flag.IsItem))
            return;

        if (HasItemOnGround) {
            if (ItemOnGrpund.actor == otherActor)
                return;

            var dits1 = (ItemOnGrpund.actor.worldPosition - m_character.actor.worldPosition).LengthSquared();
            var dits2 = (otherActor.worldPosition - m_character.actor.worldPosition).LengthSquared();
            if (dits1 >= dits2)
                return;
        }

        var newItem = m_GetItemFromTrigger(otherActor);
        if (newItem == null)
            return;

        Console.WriteLine(newItem.actor.Name);
        ItemOnGrpund = newItem;
    }

    private void m_HandleItemExit(Actor otherActor) {
        if (!otherActor.Has(Flag.IsItem))
            return;

        if (HasItemOnGround)
            return;

        var newItem = m_GetItemFromTrigger(otherActor);
        if (newItem == null)
            return;

        if (ItemOnGrpund == newItem) {
            ItemOnGrpund = null;
            Console.WriteLine("null");
        }
    }

    private void m_HandleItemDamaged(Actor otherActor)
    {
        if (!otherActor.Has(Flag.IsItem))
            return;

        var OnFlyItemComponent = otherActor.GetComponent<Item>();

        if (OnFlyItemComponent == null)
            return;

        if(OnFlyItemComponent.OnFly)
            AddDamage(55);

        Console.WriteLine("m_HandleItemDamaged");
    }

    private void m_HandleDropTargetEnter(Actor otherActor) {
        if (!otherActor.Has(Flag.IsDropTarget))
            return;

        var dropTarget = otherActor.GetComponent<DropTarget>();
        if (dropTarget == null)
            return;

        DropType = dropTarget.dropType;
    }

    private void m_HandleDropTargetExit(Actor otherActor) {
        if (!otherActor.Has(Flag.IsDropTarget))
            return;

        DropType = ItemType.None;
    }

    private Item m_GetItemFromTrigger(Actor otherActor) {
        var retarget = otherActor.GetComponent<Retarget>();
        if (retarget == null || retarget.target == null)
            return null;

        if (!retarget.target.Has(Flag.IsItem))
            return null;

        return retarget.target.GetComponent<Item>();
    }

    private void m_RotateBodyToViewDirection() {
        var forward = ViewDirection != Vector3.Zero ? ViewDirection : actor.forward;

        var bodyRotation = Quaternion.CreateRotationFromAToB(Vector3.Backward, forward, Vector3.Up);
        bodyRotation = Quaternion.CreateFromEulerAngles(bodyRotation.ToEulerAngles().deg());
        m_body.localRotationQ = Quaternion.Lerp(m_body.localRotationQ, bodyRotation, 0.3f);
    }
    public override void OnCollisionExit(Actor otherActor) {}

    private void m_DropParticle(Vector3 pos)
    {
        var particle = m_particlePrefab.Instanciate().GetComponent<AttractedParticle>();
        if (particle == null)
            throw new NullFieldException(this, $"m_particlePrefab not contains {nameof(AttractedParticle)}");
        
        float characterScale = m_character.actor.localScale.Y / 2;
        float randFloat = (float)rnd.NextDouble();
        float deltaHeight = randFloat * characterScale * 2;
        float height = (deltaHeight - characterScale);
        Vector3 particlePos = pos + new Vector3(0, height, 0);
        
        var direction = Vector3.Forward.RotateY(rnd.Next(1, 361));
        
        /*var impulse = particleDropImpulse;*///particleDropImpulse * 0.5f + (float)rnd.NextDouble() / 2;
        var impulse = particleDropImpulse * 0.5f + (float)rnd.NextDouble() / 2;
        
        particle.actor.worldPosition = particlePos;
        //particle.rigidbody.AddImpulse(direction * impulse);
        
        particle.target = SceneData.playerCharacter;
    }
}
