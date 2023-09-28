using System;
using System.IO;
using System.Diagnostics;

using Engine;

[GUID("7757cd16-db34-4221-887d-02a928d4100c")]
public class FireBinTest : CSComponent
{
    public Actor testActor;

    public bool bt_run = false;

    // TODO: Если изменяется тип поля, то происходит ошибка.
    public float value = 1;

    public override void OnInit()
    {
        actor.Flags |= Flag.IsDropTarget;
    }

    public override void OnUpdate()
    {
        if (bt_run)
        {
            bt_run = false;
            m_Run();
        }
    }

    private void m_Run()
    {
        var outputPath = @"C:\Users\Dmitry\Desktop\FireSave";

        Console.WriteLine("FireBinTest>");

        var fileStream = new FileStream(outputPath + @"\out.bin", FileMode.Create);

        var fbinData = new FireBin.Data();
        var serializer = new FireBin.Serializer(fbinData);
        var fbinWriter = new FireBin.Writer(fbinData);

        var saveTarget = actor.scene;

        // Stopwatch stopwatch = new Stopwatch();
        // stopwatch.Start();
        serializer.Serialize(saveTarget);
        // stopwatch.Stop();
        fbinWriter.Write(fileStream);

        fileStream.Close();

        // TimeSpan elapsedTime = stopwatch.Elapsed;
        // Console.WriteLine($"elapsedTime: {elapsedTime.TotalMilliseconds} ms");


        Console.WriteLine("<");
    }
}