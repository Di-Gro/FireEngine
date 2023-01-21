using System;
using Engine;


[GUID("1dba1ea0-9cb1-4c1d-bdf2-1acac058bcef")]
public class AttractedParticle : CSComponent {

    public Actor target = null;
    public Rigidbody rigidbody = null;

    public float minDistance = 40;
    public float maxDistance = 130;
    public float minSpeed = 50;
    public float maxSpeed = 250;

    private bool m_needDestroy = false;


    public override void OnInit() {
        if(rigidbody == null)
            throw new NullFieldException(this);
    }

    public override void OnUpdate() {
        if(m_needDestroy) {
            actor.Destroy();
            return;
        }

        if(target == null)
            return;

        var dist = (target.worldPosition - actor.worldPosition).Length();
        
        if(dist <= maxDistance) {
            float t = (dist - minDistance) / (maxDistance - minDistance);

            var direction = (target.worldPosition - actor.worldPosition).Normalized();
            var velosity = direction * maxSpeed * (1 - t);

            rigidbody.SetLinearVelocityClamped(velosity);
        }
    }

    public override void OnTriggerEnter(Actor otherActor, in Contact contact) {
        if (!otherActor.Has(Flag.Player))
            return;

        var retarget = otherActor.GetComponent<Retarget>();
        if (retarget == null || retarget.target == null)
            return;

        if (!retarget.target.Has(Flag.Player))
            return;

        var player = retarget.target.GetComponent<Player>();
        if (player == null)
            return;

        player.AddEnergy(1);
        m_needDestroy = true;
    }

}