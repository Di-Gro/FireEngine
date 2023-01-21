using System;

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

    private int m_health = 100;

    private Character m_character;

    private Vector3 m_handsPosition;


    public override void OnInit() {
        if (m_hands == null || m_itemSlot == null)
            throw new NullFieldException(this);

        m_character = actor.GetComponentInChild<Character>();
        if (m_character == null)
            throw new NullFieldException(this, "Character not found");

        m_character.TriggerEnterEvent += OnTriggerEnter;
        m_character.TriggerExitEvent += OnTriggerExit;

        m_handsPosition = m_hands.localRotation;
    }

    public override void OnDestroy() {
        if (!m_character.IsDestroyed) {
            m_character.TriggerEnterEvent -= OnTriggerEnter;
            m_character.TriggerExitEvent -= OnTriggerExit;
        }
    }

    public override void OnUpdate() {
        m_RotateBodyToViewDirection();
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

    public void Death() {
        if (m_health == 0)
            actor.Destroy();
    }

    public void AddDamage(int damage) {
        if (m_health > 0)
            m_health -= damage;
        else
            m_health = 0;
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

    public override void OnTriggerExit(Actor otherActor) {
        m_HandleItemExit(otherActor);
        m_HandleDropTargetExit(otherActor);
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

        if (!HasItemOnGround)
            return;

        var newItem = m_GetItemFromTrigger(otherActor);
        if (newItem == null)
            return;

        if (ItemOnGrpund == newItem) {
            ItemOnGrpund = null;
            Console.WriteLine("null");
        }
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

    private void m_ThowObjectAnimation() {
        //m_pickupedSlot.GetChild(0).localRotation.Lerp((m_pickupedSlot.GetChild(0).localRotation.X, (m_pickupedSlot.GetChild(0).localRotation.Y + 90.0f), m_pickupedSlot.GetChild(0).localRotation.Z), 10);
        m_itemSlot.GetChild(0).worldRotationQ.SetY(m_itemSlot.GetChild(0).worldRotationQ.Y + 90.0f);
    }
}
