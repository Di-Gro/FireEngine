
using Engine;

[GUID("da3da1f7-6c7a-4b56-913e-26cde5e2c7f7")]
public class DropTarget : CSComponent {

    public ItemType dropType = ItemType.Trash;

    public override void OnInit() {
        actor.Flags |= Flag.IsDropTarget;
    }
    
}