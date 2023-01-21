using System;
using System.Timers;
using System.Collections.Generic;

using Engine;

[GUID("468eb0fe-6b39-47fd-bfbd-983518874dfc")]
public class Utilizer : CSComponent {

    public float extractionImpulse = 100;
    public float dispertion = 30;

    public int minParticles = 3;
    public int maxParticles = 8;
    public float minExtrudeTime = 400;
    public float maxExtrudeTime = 800;

    [Open] private Prefab m_particlePrefab;
    [Space]
    [Open] private Rigidbody m_platform;
    [Open] private Actor m_attractionPoint;
    [Open] private Actor m_extractionPoint;

    private HashSet<Item> m_items = new HashSet<Item>();
    private List<Item> m_itemsToRemove = new List<Item>();

    private Random random = new Random();

    private int m_waitedParticles = 0;
    private float m_particlesTime = 0;

    public override void OnInit() {
        if(m_platform == null || m_attractionPoint == null || m_particlePrefab == null || m_extractionPoint == null)
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

            m_waitedParticles += random.Next(minParticles, maxParticles);
        }

        if (Input.GetButtonDown(Key.RightButton)) {
            m_waitedParticles += random.Next(minParticles, maxParticles);
        }

        if(m_waitedParticles > 0) {
            m_particlesTime -= Game.DeltaTime;
            
            if(m_particlesTime <= 0) {
                m_ExtrudeParticle();
                m_waitedParticles--;

                if (m_waitedParticles > 0) {
                    m_particlesTime = minExtrudeTime + (maxExtrudeTime - minExtrudeTime) * (float)random.NextDouble();
                }
            }
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

    private void m_ExtrudeParticle() {
        var particle = m_particlePrefab.Instanciate().GetComponent<AttractedParticle>();
        if (particle == null)
            throw new NullFieldException(this, $"m_particlePrefab not contains {nameof(AttractedParticle)}");

        // var direction = (m_extractionPoint.worldPosition - m_attractionPoint.worldPosition).Normalized();
        var direction = m_extractionPoint.forward;
        
       
        float randAngle1 = (float)random.NextDouble();
        float randAngle2 = (float)random.NextDouble();

        var right = direction.Cross(Vector3.Up);

        var rotation = Quaternion.CreateFromAxisAngle(Vector3.Up, randAngle1 * dispertion)
                     * Quaternion.CreateFromAxisAngle(right, randAngle2 * dispertion);

        direction = direction.Rotate(rotation);

        var impulse = extractionImpulse * 0.5f + (float)random.NextDouble() / 2;

        particle.actor.worldPosition = m_extractionPoint.worldPosition;
        particle.rigidbody.AddImpulse(direction * impulse);

        particle.target = SceneData.playerCharacter;
    }

}