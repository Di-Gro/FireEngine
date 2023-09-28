using System;
using System.IO;
using System.Collections.Generic;
using System.Text;

using YamlWriter = FireYaml.FireWriter;
//using PointerType = System.Int32;

namespace FireBin {
    public struct NamedList {
        public static readonly int Size = sizeof(byte) + Pointer.Size * 3;

        public StructType structType;
        public Pointer namesPtr;
        public Pointer scriptIdPtr;
        public Pointer basePtr;

        public Names names;
        public PtrList fields;
        public PtrList extra;
    }

    public struct Names {
        public FireBin.Data data;

        public int namesSign;

        public PtrList names;
        public PtrList extra;

        public int NamesCount => names.Count;
        public int ExtraCount => extra.Count;

        public string GetName(int index) => m_GetString(names[index]);
        public string GetExtra(int index) => m_GetString(extra[index]);

        private string m_GetString(Pointer strPtr) {
            var stringsArea = data[AreaId.Strings];
            return stringsArea.SaveOffset(strPtr.offset, (r, w) => r.ReadString());
        }
    }

    public struct PtrList {
        public FireBin.Data data;

        public Pointer begin;
        public int Count { get; set; }

        public Pointer this[int index] {
            get {
                var area = data[begin.areaId];
                var offset = begin.offset + index * Pointer.Size;

                return area.ReadPointer(offset);
            }
        }
    }

    public struct Scalar {
        public ScalarType scalarType;
        public int valueOffset;

        public object value;
    }

    public class Data {
        private static readonly Type[] s_scalarTypes = new Type[(int)ScalarType._Count] {
            typeof(bool),
            typeof(byte) ,
            typeof(char),
            typeof(decimal) ,
            typeof(double) ,
            typeof(short) ,
            typeof(int) ,
            typeof(long) ,
            typeof(sbyte) ,
            typeof(float) ,
            typeof(ushort) ,
            typeof(uint) ,
            typeof(ulong),
        };
        private static readonly Dictionary<Type, ScalarType> s_type_scalar = new Dictionary<Type, ScalarType>() {
            { typeof(bool), ScalarType.Bool },
            { typeof(byte), ScalarType.Byte },
            { typeof(char), ScalarType.Char },
            { typeof(decimal), ScalarType.Decimal },
            { typeof(double), ScalarType.Double },
            { typeof(short), ScalarType.Short },
            { typeof(int), ScalarType.Int },
            { typeof(long), ScalarType.Long },
            { typeof(sbyte), ScalarType.SByte },
            { typeof(float), ScalarType.Float },
            { typeof(ushort), ScalarType.UShort },
            { typeof(uint), ScalarType.UInt },
            { typeof(ulong), ScalarType.ULong }
        };
        private static readonly Action<BinaryWriter, object>[] s_scalarWriters = new Action<BinaryWriter, object>[] {
            (writer, obj) => writer.Write((bool)obj),
            (writer, obj) => writer.Write((byte)obj),
            (writer, obj) => writer.Write((char)obj),
            (writer, obj) => writer.Write((decimal)obj),
            (writer, obj) => writer.Write((double)obj),
            (writer, obj) => writer.Write((short)obj),
            (writer, obj) => writer.Write((int)obj),
            (writer, obj) => writer.Write((long)obj),
            (writer, obj) => writer.Write((sbyte)obj),
            (writer, obj) => writer.Write((float)obj),
            (writer, obj) => writer.Write((ushort)obj),
            (writer, obj) => writer.Write((uint)obj),
            (writer, obj) => writer.Write((ulong)obj)
        };
        private static readonly Func<BinaryReader, object>[] s_scalarReaders = new Func<BinaryReader, object>[] {
            (r) => r.ReadBoolean(),
            (r) => r.ReadByte(),
            (r) => r.ReadChar(),
            (r) => r.ReadDecimal(),
            (r) => r.ReadDouble(),
            (r) => r.ReadInt16(),
            (r) => r.ReadInt32(),
            (r) => r.ReadInt64(),
            (r) => r.ReadSByte(),
            (r) => r.ReadSingle(),
            (r) => r.ReadUInt16(),
            (r) => r.ReadUInt32(),
            (r) => r.ReadUInt64()
        };

        public Dictionary<int, int> m_namesOffsets = new Dictionary<int, int>();
        public Dictionary<int, int> m_assetRefOffsets = new Dictionary<int, int>();

        public List<Reference> references = new List<Reference>();
        public Area[] areas;

        public Data() {
            areas = new Area[(int)AreaId._Count] {
                new Area(AreaId.Structs, references),
                new Area(AreaId.Names, references),
                new Area(AreaId.Scalars, references),
                new Area(AreaId.AssetRefs, references),
                new Area(AreaId.Refs, references),
                new Area(AreaId.Strings, references),
            };
        }

        public Area this[AreaId index] => areas[(int)index];

        public int GetNamesCount(int namesOffset) {
            var namesArea = this[AreaId.Names];

            var lastOffset = namesArea.Offset;

            namesArea.Offset = namesOffset + sizeof(int);
            var count = namesArea.reader.ReadInt32();

            namesArea.Offset = lastOffset;

            return count;
        }

        public int GetExtraNamesCount(int namesOffset) {
            var namesArea = this[AreaId.Names];

            var lastOffset = namesArea.Offset;

            namesArea.Offset = namesOffset + sizeof(int) * 2;
            var count = namesArea.reader.ReadInt32();

            namesArea.Offset = lastOffset;

            return count;
        }

        public Address AddAssetRef(string assetId) {
            var assetRefArea = this[AreaId.AssetRefs];
            var assetIdHash = Guid.Parse(assetId).GetHashCode();

            if (m_assetRefOffsets.ContainsKey(assetIdHash))
                return new Address() { area = assetRefArea, offset = m_assetRefOffsets[assetIdHash] };

            var offset = assetRefArea.Offset;

            //Console.WriteLine($"{assetIdHash}: {offset}: {assetId}");

            m_assetRefOffsets[assetIdHash] = offset;
            assetRefArea.writer.Write(assetId);

            return new Address() { area = assetRefArea, offset = offset };
        }

        public int AddNames(Type type, List<FireYaml.Field> fields, Engine.SerializerBase serializer) {
            //var typeId = type.FullName.GetHashCode();
            var extraNames = serializer.GetNamesOfExtraFields();
            var namesSign = GetNamesSign(type, fields, extraNames);

            if (m_namesOffsets.ContainsKey(namesSign))
                return m_namesOffsets[namesSign];

            var namesArea = this[AreaId.Names];
            var stringsArea = this[AreaId.Strings];
            var namesOffset = namesArea.Offset;

            m_namesOffsets[namesSign] = namesOffset;
            namesArea.writer.Write(namesSign);
            namesArea.writer.Write(fields.Count);
            namesArea.writer.Write(extraNames != null ? extraNames.Count : 0);

            foreach (var field in fields) {
                namesArea.WritePointer(stringsArea, stringsArea.Offset);
                stringsArea.writer.Write(field.name);
            }
            if (extraNames != null) {
                foreach (var name in extraNames) {
                    namesArea.WritePointer(stringsArea, stringsArea.Offset);
                    stringsArea.writer.Write(name);
                }
            }
            return namesOffset;
        }

        public void CollectNames() {
            m_namesOffsets.Clear();

            var namesArea = this[AreaId.Names];

            var prevPos = namesArea.Offset;
            namesArea.Offset = 0;

            while (namesArea.Offset < namesArea.Length) {
                var offset = namesArea.Offset;

                var namesSign = namesArea.reader.ReadInt32();
                var count = namesArea.reader.ReadInt32();
                var extraCount = namesArea.reader.ReadInt32();

                for (int i = 0; i < count + extraCount; i++)
                    namesArea.ReadPointer();

                m_namesOffsets[namesSign] = offset;
            }
            namesArea.Offset = prevPos;
        }

        public void CollectAssetRefs() {
            m_assetRefOffsets.Clear();

            var assetRefArea = this[AreaId.AssetRefs];

            var prevPos = assetRefArea.Offset;
            assetRefArea.Offset = 0;

            while (assetRefArea.Offset < assetRefArea.Length) {
                var offset = assetRefArea.Offset;

                var assetId = assetRefArea.reader.ReadString();
                var assetIdHash = Guid.Parse(assetId).GetHashCode();

                //Console.WriteLine($"{assetIdHash}: {offset}: {assetId}");

                m_assetRefOffsets[assetIdHash] = offset;
            }
            assetRefArea.Offset = prevPos;
        }

        public static int GetNamesSign(Type type, List<FireYaml.Field> fields, List<string> extraNames) {
            string str = type.FullName;

            foreach (var field in fields)
                str += $"_{field.name}";

            if (extraNames != null) {
                foreach (var name in extraNames)
                    str += $"-{name}";
            }

            return str.GetHashCode();
        }

        public static ScalarType? GetScalarType(Type type) {
            if (!s_type_scalar.ContainsKey(type))
                return null;

            return s_type_scalar[type];
        }

        public static Type GetType(ScalarType scalarType) {
            return s_scalarTypes[(int)scalarType];
        }

        public StructType ReadStructType() {
            var structArea = this[AreaId.Structs];

            var structTypeIndex = structArea.reader.ReadByte();

            if (structTypeIndex < 0 || structTypeIndex >= (byte)StructType._Count)
                throw new FireBinException($"Invalid struct type index: {structTypeIndex}");

            return (StructType)structTypeIndex;
        }

        public string ReadScriptId(Pointer namedListPtr) {
            namedListPtr.Check(AreaId.Structs);

            var structArea = this[AreaId.Structs];
            return structArea.SaveOffset(namedListPtr.offset, (r, w) => ReadScriptId());
        }

        public string ReadScriptId() {
            var structArea = this[AreaId.Structs];

            var structType = ReadStructType();
            if (structType != StructType.NamedList)
                throw new FireBinException($"Struct {structType} don't have a GUID.");

            structArea.Offset += Pointer.Size; // namesPtr

            var scriptIdPtr = structArea.ReadPointer();
            var scriptId = ReadAssetRef(scriptIdPtr);

            return scriptId;
        }

        public string ReadAssetRef(Pointer assetRefPtr) {
            assetRefPtr.Check(AreaId.AssetRefs);

            var assetRefsArea = this[AreaId.AssetRefs];

            var assetRef = assetRefsArea.SaveOffset(assetRefPtr.offset, (r, w) => r.ReadString());
            return assetRef;
        }

        public Pointer ReadReference(Pointer refPtr) {
            refPtr.Check(AreaId.Refs);

            var refsArea = this[AreaId.Refs];

            var structPtr = refsArea.ReadPointer(refPtr.offset);
            structPtr.Check(AreaId.Structs);

            return structPtr;
        }

        public NamedList ReadNamedList(Pointer structPtr) {
            structPtr.Check(AreaId.Structs);

            var structArea = this[AreaId.Structs];

            return structArea.SaveOffset(structPtr.offset, (r, w) => ReadNamedList());
        }

        public NamedList ReadNamedList() {
            var structArea = this[AreaId.Structs];
            var header = new NamedList();

            header.structType = ReadStructType();
            header.namesPtr = structArea.ReadPointer();
            header.scriptIdPtr = structArea.ReadPointer();
            header.basePtr = structArea.ReadPointer();

            header.names = ReadNames(header.namesPtr);
            header.fields = ReadPtrList(AreaId.Structs, header.names.NamesCount);
            header.extra = ReadPtrList(AreaId.Structs, header.names.ExtraCount);

            return header;
        }

        public Names ReadNames(Pointer namesPtr) {
            namesPtr.Check(AreaId.Names);

            var namesArea = this[AreaId.Names];

            return namesArea.SaveOffset(namesPtr.offset, (r, w) => ReadNames());
        }

        public Names ReadNames() {
            var namesArea = this[AreaId.Names];

            var namesSign = namesArea.reader.ReadInt32();
            var namesCount = namesArea.reader.ReadInt32();
            var extraCount = namesArea.reader.ReadInt32();
            var namesList = ReadPtrList(AreaId.Names, namesCount);
            var extraList = ReadPtrList(AreaId.Names, extraCount);

            var header = new Names();

            header.data = this;
            header.namesSign = namesSign;
            header.names = namesList;
            header.extra = extraList;

            return header;
        }

        public PtrList ReadList(Pointer listPtr) {
            listPtr.Check(AreaId.Structs);

            var structArea = this[AreaId.Structs];

            return structArea.SaveOffset(listPtr.offset, (r, w) => ReadList());
        }

        public PtrList ReadList() {
            var structArea = this[AreaId.Structs];

            var structType = ReadStructType();
            if (structType != StructType.List)
                throw new FireBinException($"Struct type: '{structType}' is not a '{StructType.List}'.");

            var count = structArea.reader.ReadInt32();
            var values = ReadPtrList(AreaId.Structs, count);

            return values;
        }

        public PtrList ReadPtrList(AreaId areaId, int count) {
            var area = this[areaId];
            var header = new PtrList();

            header.data = this;
            header.begin = new Pointer() { areaId = areaId, offset = area.Offset };
            header.Count = count;

            area.Offset += count * Pointer.Size;

            return header;
        }

        public void WriteScalar(Type type, object obj) {
            var scalarType = GetScalarType(type);
            if (scalarType == null)
                throw new FireBinException($"Type: {type.Name} is not a scalar type.");

            var scalarsArea = this[AreaId.Scalars];
            var scalarWriter = s_scalarWriters[(int)scalarType];

            scalarsArea.writer.Write((byte)scalarType);
            scalarWriter.Invoke(scalarsArea.writer, obj);
        }

        public T ReadScalar<T>(Pointer scalarPtr) {
            scalarPtr.Check(AreaId.Scalars);

            var scalarsArea = this[AreaId.Scalars];

            var scalar = scalarsArea.SaveOffset(scalarPtr.offset, (r, w) => ReadScalar());

            if(typeof(T) != scalar.value.GetType())
                throw new FireBinException($"Incorrect scalar type: '{scalar.value.GetType()}', when expected type: '{typeof(T)}'.");

            return (T)scalar.value;
        }

        public Scalar ReadScalar() {
            var scalarsArea = this[AreaId.Scalars];

            var typeIndex = scalarsArea.reader.ReadByte();
            var valueOffset = scalarsArea.Offset;

            if (typeIndex < 0 || typeIndex >= (byte)ScalarType._Count)
                throw new FireBinException($"Incorrect scalar type index: {typeIndex}.");

            var scalarReader = s_scalarReaders[typeIndex];
            var value = scalarReader.Invoke(scalarsArea.reader);

            var header = new Scalar();
            header.scalarType = (ScalarType)typeIndex;
            header.valueOffset = valueOffset;
            header.value = value;

            return header;
        }

        public ValueType? GetValueType(Pointer ptr) {
            switch (ptr.areaId) {
                case AreaId.Structs:
                    var structsArea = this[AreaId.Structs];
                    var structType = structsArea.SaveOffset(ptr.offset, (r, w) => ReadStructType());

                    switch (structType) {
                        case StructType.List:       return ValueType.List;
                        case StructType.NamedList:  return ValueType.NamedList;
                        case StructType.Enum:       return ValueType.Enum;
                        case StructType.Vector2:    return ValueType.Vector2;
                        case StructType.Vector3:    return ValueType.Vector3;
                        case StructType.Quaternion: return ValueType.Quaternion;
                    }
                    break;
                case AreaId.Scalars:    return ValueType.Scalar;
                case AreaId.AssetRefs:  return ValueType.AssetRef;
                case AreaId.Refs:       return ValueType.Ref;
                case AreaId.Strings:    return ValueType.String;
            }
            return null;
        }

        public static ValueType? GetValueType(Type type) {
            if (type.IsEnum)
                return ValueType.Enum;

            if (type == typeof(string))
                return ValueType.String;

            if (FireBin.Data.GetScalarType(type) != null)
                return ValueType.Scalar;

            if (type == typeof(Engine.Vector2))
                return ValueType.Vector2;

            if (type == typeof(Engine.Vector3))
                return ValueType.Vector3;

            if (type == typeof(Engine.Quaternion))
                return ValueType.Quaternion;

            if (YamlWriter.NeedSaveAsLink(type))
                return ValueType.Ref;

            if (YamlWriter.IsList(type))
                return ValueType.List;

            if (YamlWriter.IsAsset(type))
                return ValueType.AssetRef;

            if (type.IsClass || type.IsValueType)
                return ValueType.NamedList;

            return null;
        }

    }
}
