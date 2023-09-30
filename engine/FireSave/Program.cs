using System.Reflection;
using FireBinEditor;

var outputPath = @"C:\Users\Dmitry\Desktop\FireSave";

Console.WriteLine("FireBinTest>");

FireYaml.AssetStore.Instance = new FireYaml.AssetStore();
FireYaml.AssetStore.Instance.CollectTypes(new Assembly[] { 
    Assembly.GetAssembly(typeof(Program))
});

bool serialize = true;
bool deserialize = true;

if (serialize) {
    var fileStream = new FileStream(outputPath + @"\out.bin", FileMode.Create);

    var fbinData = new FireBin.Data();
    var serializer = new FireBin.Serializer(fbinData);
    var fbinWriter = new FireBin.FileWriter(fbinData);

    //var comp1 = new TestComponent1();
    //var comp2 = new TestComponent2();

    //comp1.comp2Ref = comp2;
    //comp1.material = new Engine.StaticMaterial().LoadFromAsset("418dccc8-cf33-4f3e-b401-ae42cb9cc7f7");



    //var testClass = new TestClass();
    //testClass.floatValue = 56.475f;
    //testClass.uintValue = 8;

    //testClass.lists = new TestClass_Lists();
    //testClass.lists.intList = new List<int>() { 11, 12, 13 };
    //testClass.lists.compList.Add(comp1);
    //testClass.lists.compList.Add(comp2);

    //testClass.nullField = new FireBinEditor.TestClass_Scalars();

    //testClass.nullField.ByteField += 10;
    //testClass.nullField.DecimalField += 10;
    //testClass.nullField.DoubleField += 10;
    //testClass.nullField.ShortField += 10;
    //testClass.nullField.IntField += 10;
    //testClass.nullField.LongField += 10;
    //testClass.nullField.SByteField += 10;
    //testClass.nullField.FloatField += 10;
    //testClass.nullField.UShortField += 10;
    //testClass.nullField.UIntField += 10;
    //testClass.nullField.ULongField += 10;

    //var list = new List<int>() { 5, 4, 3 };

    //serializer.AddAsList(list.GetType(), list);
    //serializer.Add(testClass);
    //serializer.Add(comp1);
    //serializer.Add(comp2);
    serializer.Serialize();

    fbinWriter.Write(fileStream);

    fileStream.Close();
} 

if (deserialize) {
    var fileStream = new FileStream(outputPath + @"\out.bin", FileMode.Open);

    var fbinReader = new FireBin.FileReader();
    var fbinData = fbinReader.Read(fileStream);
    var deserializer = new FireBin.Deserializer(fbinData);

    //var comp1 = new TestComponent1();
    //var comp2 = new TestComponent2();
    //var testClass = new TestClass();

    //var list = new List<int>();
    //var ptr = new FireBin.Pointer() { areaId = FireBin.AreaId.Structs, offset = 0 };

    //deserializer.LoadAsList(list.GetType(), ptr, list);

    //deserializer.Load(testClass, 0);
    //deserializer.Load(comp1, 82);
    //deserializer.Load(comp2, 108);
    deserializer.EndLoad();

    //var instance = deserializer.Load();

    fileStream.Close();
}


Console.WriteLine("<");