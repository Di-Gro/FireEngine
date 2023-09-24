using System;
using System.IO;

using Engine;

[GUID("7757cd16-db34-4221-887d-02a928d4100c")]
public class FireBinTest : CSComponent
{

    public bool bt_run = false;

    public override void OnInit()
    {
        actor.Flags |= Flag.IsDropTarget;
    }

    public override void OnUpdate()
    {
        if(bt_run) {
            bt_run = false;
            m_Run();
        }
    }

    private void m_Run() {
        var outputPath = @"C:\Users\Dmitry\Desktop\FireSave";

        Console.WriteLine("FireBinTest>");

        // var writer = new FireBin.Writer();
        // var material = new StaticMaterial();

        // writer.WriteAsNamedList(material.GetType(), material);

        // var fileStream = new FileStream(outputPath + @"\out.bin", FileMode.Create);
        // var fileWriter = new BinaryWriter(fileStream);

        // writer.Write(fileWriter);

        // fileWriter.Close();

        Console.WriteLine("<");
    }
}