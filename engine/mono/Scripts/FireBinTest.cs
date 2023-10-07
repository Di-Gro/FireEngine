using System;
using System.IO;
using System.Diagnostics;

using Engine;

[GUID("7757cd16-db34-4221-887d-02a928d4100c")]
public class FireBinTest : CSComponent
{
    public Actor fireBinTarget;
    public Actor actorRef;

    public bool bt_run = false;
    public bool serialize = true;
    public bool deserialize = true;

    public bool useCsRefs = false;

    private string outputPath = @"C:\Users\Dmitry\Desktop\FireSave";

    private FireBin.Data fbinData = null;


    public override void OnInit()
    {
        actor.Flags |= Flag.IsDropTarget;
    }

    public override void OnUpdate() {
        if (bt_run) {
            bt_run = false;

            if (serialize)
                ToFireBin();

            if (deserialize)
                FromFireBin();
        }
    }

    public void ToFireBin() {
        // var fireDB = new Engine.AssetMenager(outputPath);

        // fireDB.Load();


        var fileStream = new FileStream(outputPath + @"\out.bin", FileMode.Create);

        fbinData = new FireBin.Data();

        var serializer = new FireBin.Serializer(fbinData);

        Stopwatch stopwatch = new Stopwatch();
        stopwatch.Start();

        // Console.WriteLine($"Serialize >");
        serializer.Serialize(fireBinTarget);
        // Console.WriteLine($"<");

        stopwatch.Stop();

        TimeSpan elapsedTime = stopwatch.Elapsed;
        Console.WriteLine($"FireBin.Serialize: {elapsedTime.TotalMilliseconds} ms");

        new FireBin.DataWriter(fbinData).Write(new BinaryWriter(fileStream));

        fileStream.Close();

        if (actorRef != null)
            actorRef.Destroy();
    }

    public void FromFireBin() {
        if (fbinData == null)
            return;

        var deserializer = new FireBin.Deserializer(fbinData, useCsRefs: useCsRefs);

        // Console.WriteLine($"Deserialize >");
        var loadedObj = deserializer.Instanciate();
        // Console.WriteLine($"<");
    }

    private void m_Run()
    {
        // var outputPath = @"C:\Users\Dmitry\Desktop\FireSave";

        // Console.WriteLine("FireBinTest>");

        // var fileStream = new FileStream(outputPath + @"\out.bin", FileMode.Create);

        // var fbinData = new FireBin.Data();
        // var serializer = new FireBin.Serializer(fbinData);
        // var fbinWriter = new FireBin.FileWriter(fbinData);

        // var saveTarget = actor.scene;

        // // Stopwatch stopwatch = new Stopwatch();
        // // stopwatch.Start();
        // serializer.Serialize(saveTarget);
        // // stopwatch.Stop();
        // fbinWriter.Write(fileStream);

        // fileStream.Close();

        // // TimeSpan elapsedTime = stopwatch.Elapsed;
        // // Console.WriteLine($"elapsedTime: {elapsedTime.TotalMilliseconds} ms");


        // Console.WriteLine("<");
    }
}