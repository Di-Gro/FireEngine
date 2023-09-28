using System.Reflection;

var outputPath = @"C:\Users\Dmitry\Desktop\FireSave";

Console.WriteLine("FireBinTest>");

FireYaml.AssetStore.Instance = new FireYaml.AssetStore();
FireYaml.AssetStore.Instance.CollectTypes(new Assembly[] { 
    Assembly.GetAssembly(typeof(Program))
});

bool serialize = false;
bool deserialize = true;

if (serialize) {
    var fileStream = new FileStream(outputPath + @"\out.bin", FileMode.Create);

    var fbinData = new FireBin.Data();
    var serializer = new FireBin.Serializer(fbinData);
    var fbinWriter = new FireBin.Writer(fbinData);

    var saveTarget = new FireBinEditor.TestClass();
    //saveTarget.floatValue = 56.475f;
    //saveTarget.uintValue = 8;

    saveTarget.nullField = new FireBinEditor.TestClass_Scalars();

    saveTarget.nullField.ByteField += 10;
    saveTarget.nullField.DecimalField += 10;
    saveTarget.nullField.DoubleField += 10;
    saveTarget.nullField.ShortField += 10;
    saveTarget.nullField.IntField += 10;
    saveTarget.nullField.LongField += 10;
    saveTarget.nullField.SByteField += 10;
    saveTarget.nullField.FloatField += 10;
    saveTarget.nullField.UShortField += 10;
    saveTarget.nullField.UIntField += 10;
    saveTarget.nullField.ULongField += 10;

    serializer.Serialize(saveTarget);
    fbinWriter.Write(fileStream);

    fileStream.Close();
} 

if (deserialize) {
    var fileStream = new FileStream(outputPath + @"\out.bin", FileMode.Open);

    var fbinReader = new FireBin.Reader();
    var fbinData = fbinReader.Read(fileStream);
    var deserializer = new FireBin.Deserializer(fbinData);

    deserializer.Deserialize();

    fileStream.Close();
}



Console.WriteLine("<");