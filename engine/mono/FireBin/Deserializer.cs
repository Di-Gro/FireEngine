using System;
using System.IO;
using System.Collections;
using System.Collections.Generic;
using System.Text;

using YamlWriter = FireYaml.FireWriter;
using System.Numerics;

/// TODO: CsRefs 
/// TODO: InstanciateComponent
/// TODO: решить нужны ли m_assetInst, m_writeIDs, IFile.assetInstance, IFile.fileId
/// TODO: assetInstance
/// 
/// TODO: Сделать проверку на тип ассета. 
/// ----: Сейчас Mesh попробует загрузиться в Material. 
/// 
/// ASK: Что если genericType в списке это интерфейс?
/// ---: Если genericType это интерфейс, то такой список не сохранится. 
/// 
/// ASK: Что если genericType в списке это структура?
/// ---: Тогда список сохранится, если у структуры есть GUID и new().
/// 
/// ASK: Что если genericType в списке это базовый класс?
/// ---: Тогда список сохранится, если у базового класса есть GUID и new().
/// ---: Элементы списка у классов которых нет GUID и new(), сохранятся как null.
/// ---: По GUID будет создан экземпляр нужного типа и записан в ссылку на базовый класс.
/// 
/// ASK: Что если поле класса будет ссылкой на базовый класс?
/// ---: По GUID будет создан экземпляр нужного типа и записан в ссылку на базовый класс.
/// 
/// ASK: Какие типы могут быть сохранены? 
/// ---: 1) Component-ы и Actor-ы. Если они являются полями или эллементами списка, 
/// ---: то сохраняются как ссылки и востанавливают связи после загрузки. 
/// ---: 2) string, Vector2, Vector3, Vector4, Quaternion.
/// ---: 3) List<genericType>, если genericType тоже может быть сохранен, кроме списка. 
/// ---:    То есть не может быть вложенных списков. 
/// ---: 4) Примитивные типы, кроме decimal.
/// ---: 5) Перечисления.
/// ---: 5) Классы и структуры, у которых есть GUID и new().
/// ---: 6) StaticAsset-ы. После загрузки нужные ассеты будут загружены в память.

namespace FireBin {
    public class FireBinException : Exception {
        public FireBinException(string msg) : base(msg) { }
    }

    public class Deserializer : Engine.IDeserializer {
        private struct Ref {
            public Pointer refPtr;
            public FireYaml.Field field;

            public Ref(Pointer refPtr, FireYaml.Field field) { 
                this.refPtr = refPtr;
                this.field = field;
            }
        }

        public event Action EndLoadEvent;

        public DataReader Reader { get; private set; }

        public delegate object LoadAsDelegate(Type type, Pointer dataPtr, object target = null);

        private readonly LoadAsDelegate[] m_deserializers;

        private Dictionary<Pointer, object> m_loadedStructs = new Dictionary<Pointer, object>();
        private List<Ref> m_refs = new List<Ref>();
        private List<FireYaml.IAsset> m_assets = new List<FireYaml.IAsset>();

        private string m_typeConflicts = "";
        private string m_nameConflicts = "";

        private bool m_useCsRefs = false;

        public Deserializer(FireBin.Data data, bool useCsRefs = false) {
            Reader = new FireBin.DataReader(data);
            m_useCsRefs = useCsRefs;

            m_deserializers = new LoadAsDelegate[(int)BinType._Count] {
                LoadAsList,
                LoadAsNamedList,
                LoadAsEnum,
                LoadAsAssetRef,
                (Type type, Pointer dataPtr, object target) => null,
                (Type type, Pointer dataPtr, object target) => Reader.ReadScalar(dataPtr).value,
                (Type type, Pointer dataPtr, object target) => Reader.ReadString(dataPtr),
                LoadAsVector2,
                LoadAsVector3,
                LoadAsQuaternion,
            };
        }

        public T Instanciate<T>() where T : new() {
            return (T)Instanciate();
        }

        public object Instanciate() {
            m_GetFirstStructInfo(out var type, out var structPtr);

            var target = CreateInstance(type);

            LoadAsNamedList(type, structPtr, target);

            if (YamlWriter.IsAsset(type))
                m_assets.Add((FireYaml.IAsset)target);

            EndLoad();

            return target;
        }

        public void InstanciateTo(object target) {
            m_GetFirstStructInfo(out var type, out var structPtr);

            LoadAsNamedList(type, structPtr, target);

            if(YamlWriter.IsAsset(type))
                m_assets.Add((FireYaml.IAsset)target);

            EndLoad();
        }

        public void InstanciateToWithoutLoad(object target) {
            m_GetFirstStructInfo(out var type, out var structPtr);

            LoadAsNamedList(type, structPtr, target);
            EndLoad();
        }

        private void m_GetFirstStructInfo(out Type type, out Pointer structPtr) {
            structPtr = new Pointer { areaId = AreaId.Structs, offset = 0 };

            var scriptId = Reader.ReadScriptId(structPtr);
            if (scriptId == "")
                throw new FireBinException("Struct not contains scriptId");

            type = GetTypeOf(scriptId);
        }

        public Engine.Component InstanciateComponent(Engine.Actor actor) {
            var structPtr = new Pointer { areaId = AreaId.Structs, offset = 0 };

            var component = new Engine.ActorSerializer().LoadComponent(this, actor, structPtr);
            EndLoad();

            return component;
        }

        public void EndLoad() {
            m_ResolveRefs();

            foreach (var asset in m_assets)
                asset.LoadAsset();
#if DETACHED
#else
#endif
            EndLoadEvent?.Invoke();
        }

        public object LoadAsNamedList(Type type, Pointer dataPtr, object target = null) {
            var serializer = YamlWriter.GetSerializer(type);

            var valueObj = target != null ? target : CreateInstance(type);
            var data = Reader.ReadNamedList(dataPtr);

            if (Data.ThrowBinType(type) == BinType.Ref)
                m_loadedStructs.Add(dataPtr, valueObj);

            if (serializer.NeedIncludeBase(type) && data.basePtr.offset != Pointer.NullOffset)
                valueObj = LoadAsNamedList(type.BaseType, data.basePtr, valueObj);

            for (int i = 0; i < data.fields.Count; i++) {
                var fieldName = data.names.GetName(i);
                var valuePtr = data.fields[i].Value;

                var field = GetSupportedField(fieldName, valuePtr, type, valueObj, serializer);
                if (field == null)
                    continue;

                // Console.WriteLine($"names[{i}]: {fieldName}");

                if (valuePtr.offset == Pointer.NullOffset) {
                    field.SetValue(null);
                    continue;
                }
                var binType = Data.ThrowBinType(field.type);
                var valueType = field.type;

                if (binType == BinType.Ref) {
                    m_refs.Add(new Ref(valuePtr, field));
                    field.SetValue(null);
                    continue;
                }
                if (binType == BinType.NamedList) {
                    var scriptId = Reader.ReadScriptId(valuePtr);
                    valueType = GetTypeOf(scriptId);
                }
                               
                var fieldReader = m_GetLoader(binType);
                var fieldValue = fieldReader.Invoke(valueType, valuePtr);

                field.SetValue(fieldValue);
            }
            if (data.extraFields.Count > 0)
                serializer.ReadExtraFields(this, valueObj, data.extraFields);

            return valueObj;
        }

        public object LoadAsList(Type type, Pointer dataPtr, object target = null) {
            var valueObj = target != null ? target : CreateInstance(type);
            var data = Reader.ReadList(dataPtr);

            var list = valueObj as IList;
            list.Clear();

            var genericType = type.GetGenericArguments()[0];
            var genericBinType = Data.ThrowBinType(genericType);
            var itemReader = m_GetLoader(genericBinType);

            for (int i = 0; i < data.Count; i++) {
                var valuePtr = data[i].Value;
                if (valuePtr.offset == Pointer.NullOffset) {
                    list.Add(null);
                    continue;
                }
                var valueType = genericType;
                var valueBinType = Reader.ThrowBinType(valuePtr);

                if (genericBinType != valueBinType) {
                    m_WriteTypeConflict(genericBinType, valueBinType, $"List<{genericType.Name}>[{i}]");
                    list.Add(null);
                    continue;
                }
                if (genericBinType == BinType.Ref) {
                    m_refs.Add(new Ref(valuePtr, new FireYaml.Field(list, genericType, i)));
                    list.Add(null);
                    continue;
                }                
                if (genericBinType == BinType.NamedList) {
                    var scriptId = Reader.ReadScriptId(valuePtr);
                    valueType = GetTypeOf(scriptId);
                }
                var fieldValue = itemReader.Invoke(valueType, valuePtr);

                list.Add(fieldValue);
            }
            return valueObj;
        }

        public object LoadAsEnum(Type type, Pointer dataPtr, object target = null) {
            var binEnum = Reader.ReadEnum(dataPtr);

            var strValue = Enum.GetName(type, binEnum.intValue);
            if (strValue == null) {
                m_WriteEnumIntConflict(type, binEnum.intValue, binEnum.strValue);
                return Enum.ToObject(type, Enum.GetNames(type)[0]);
            }
            if(strValue != binEnum.strValue)
                m_WriteEnumStrConflict(type, binEnum.intValue, binEnum.strValue, strValue);

            return Enum.ToObject(type, binEnum.intValue);
        }

        public object LoadAsAssetRef(Type type, Pointer dataPtr, object target = null) {
            var valueObj = target != null ? target : CreateInstance(type);

            var fieldName = nameof(FireYaml.IAsset.assetId);
            var serializer = new Engine.SerializerBase();
            var field = YamlWriter.GetField(fieldName, type, valueObj, serializer);

            var assetId = Reader.ReadAssetRef(dataPtr);
            field.SetValue(assetId);

            m_assets.Add((FireYaml.IAsset)field.Instance);

            return valueObj;
        }

        public object LoadAsVector2(Type type, Pointer dataPtr, object target = null) {
            var values = Reader.ReadVector(dataPtr, 2);

            return new Engine.Vector2(values[0], values[1]);
        }

        public object LoadAsVector3(Type type, Pointer dataPtr, object target = null) {
            var values = Reader.ReadVector(dataPtr, 3);

            return new Engine.Vector3(values[0], values[1], values[2]);
        }

        public object LoadAsQuaternion(Type type, Pointer dataPtr, object target = null) {
            var values = Reader.ReadVector(dataPtr, 4);

            return new Engine.Quaternion(values[0], values[1], values[2], values[3]);
        }

        private LoadAsDelegate m_GetLoader(BinType valueType) {
            return m_deserializers[(int)valueType];
        }

        public static Type GetTypeOf(string scriptId) {
            var type = Engine.GUIDAttribute.GetTypeByGuid(scriptId);
            if (type == null)
                throw new FireBinException("Type not found");

            return type;
        }

        public static object CreateInstance(Type type) {
            if (type == typeof(string))
                return "";

            var instance = Activator.CreateInstance(type, true);
            if (instance == null)
                throw new FireBinException("A parameterless constructor is required to load the object.");

            return instance;
        }

        private void m_ResolveRefs() {
            foreach(var iRef in m_refs) {
                var reference = Reader.ReadReference(iRef.refPtr);

                if (m_loadedStructs.ContainsKey(reference.to))
                    iRef.field.SetValue(m_loadedStructs[reference.to]);
                else if (m_useCsRefs)
                    iRef.field.SetValue(Engine.CppLinked.GetObjectByRef(reference.csRef));
                else
                    iRef.field.SetValue(null);
            }
        }

        private FireYaml.Field GetSupportedField(string fieldName, Pointer valuePtr, Type type, object target, Engine.SerializerBase serializer) {
            var field = YamlWriter.GetField(fieldName, type, target, serializer);
            if (field == null) {
                m_WriteNameConflict(type, fieldName);
                return null;
            }
            var fieldType = Data.GetBinType(field.type);
            if (fieldType == null)
                return null;

            if (valuePtr.offset == Pointer.NullOffset)
                return field;

            var valueType = Reader.ThrowBinType(valuePtr);

            if (valueType != fieldType) {
                m_WriteTypeConflict(fieldType.Value, valueType, fieldName);
                return null;
            }
            return field;
        }

        private void m_WriteEnumStrConflict(Type enumType, int intValue, string expectedName, string name) {
            m_nameConflicts += $"The name: '{name}' of the value at index: {intValue} in the enum: {enumType.Name} does not match the previous name: '{expectedName}'.";
        }

        private void m_WriteEnumIntConflict(Type enumType, int intValue, string expectedName) {
            m_nameConflicts += $"Enum: {enumType.Name} does not contain a value at index: '{intValue}'. The expected name is '{expectedName}'.\n";
        }

        private void m_WriteNameConflict(Type type, string fieldName) {
            m_nameConflicts += $"Field: '{fieldName}' not found in type: '{type.FullName}'.\n";
        }

        private void m_WriteTypeConflict(BinType fieldType, BinType valueType, string fieldName) {
            m_typeConflicts += $"Field '{fieldName}' has {nameof(BinType)}: '{fieldType}', but the value has type: '{valueType}'.\n";
        }

    }
}
