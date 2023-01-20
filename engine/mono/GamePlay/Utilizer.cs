using System;
using System.Collections.Generic;

using Engine;

[GUID("468eb0fe-6b39-47fd-bfbd-983518874dfc")]
public class Utilizer : CSComponent {

    [Open] private Rigidbody m_platform;
    [Open] private Actor m_attractionPoint;

    private HashSet<Item> m_items = new HashSet<Item>();
    private List<Item> m_itemsToRemove = new List<Item>();



    public override void OnInit() {
        if(m_platform == null || m_attractionPoint == null)
            throw new NullFieldException(this);

        m_platform.TriggerEnterEvent += OnTriggerEnter;
        m_platform.TriggerExitEvent += OnTriggerExit;
    }

    public override void OnDestroy() {
        if (!m_platform.IsDestroyed) {
            m_platform.TriggerEnterEvent -= OnTriggerEnter;
            m_platform.TriggerExitEvent -= OnTriggerExit;
        }
    }

    public override void OnUpdate() {
        foreach (var item in m_items) {
            item.actor.worldPosition = item.actor.worldPosition.Lerp(m_attractionPoint.worldPosition, Game.DeltaTime);
            
            var dist = (m_attractionPoint.worldPosition - item.actor.worldPosition).Length();
            var radius = item.actor.worldScale.MaxComponent() / 1.5f;

            if (dist <= radius)
                m_itemsToRemove.Add(item);
        }

        for (int i = m_itemsToRemove.Count - 1; i >= 0; i--) {
            var item = m_itemsToRemove[i];

            item.actor.Destroy();
            m_items.Remove(item);
            m_itemsToRemove.RemoveAt(i);
        }
    }

    public override void OnTriggerEnter(Actor otherActor, in Contact contact) {
        if (!otherActor.Has(Flag.IsItem))
            return;

        var newItem = m_GetItemFromTrigger(otherActor);
        if (newItem == null)
            return;

        if(m_items.Contains(newItem))
            return;

        m_items.Add(newItem);
        newItem.canPickup = false;
    }

    private Item m_GetItemFromTrigger(Actor otherActor) {
        var retarget = otherActor.GetComponent<Retarget>();
        if (retarget == null || retarget.target == null)
            return null;

        if (!retarget.target.Has(Flag.IsItem))
            return null;

        return retarget.target.GetComponent<Item>();
    }

}