
using Engine;

[GUID("df4d594d-2fe2-4902-8b8a-08232a172eb4")]
public class Retarget : CSComponent {
    public Actor target = null;

    public override void OnInit() {
        actor.Flags |= Flag.IsRetarget;
    }
}