using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;

using YamlWriter = FireYaml.FireWriter;

namespace FireBin {
    public enum StructType {
        List,
        NamedList,
        Enum,
        Vector2,
        Vector3,
        Quaternion,
        _Count,
    }

    public enum BinType {
        List,
        NamedList,
        Enum,
        AssetRef,
        Ref,
        Scalar,
        String,
        Vector2,
        Vector3,
        Quaternion,
        _Count,
    }

    public enum AreaId {
        Structs,
        Names,
        Scalars,
        AssetRefs,
        Refs,
        Strings,
        _Count,
    }

    public enum ScalarType {
        Bool,
        Byte,
        Char,
        Decimal,
        Double,
        Short,
        Int,
        Long,
        SByte,
        Float,
        UShort,
        UInt,
        ULong,
        _Count
    }

    public struct NamedList {
        public static readonly int Size = sizeof(byte) + Pointer.Size * 3;

        public FireBin.DataWriter writer;
        public int structOffset;
        public int basePtrOffset;

        public StructType structType;
        public Pointer namesPtr;
        public Pointer scriptIdPtr;
        public Pointer basePtr;

        public Names names;
        public PtrList fields;
        public PtrList extraFields;

        public void WriteBasePtr(Pointer? value) {
            writer.WritePointer(AreaId.Structs, value, basePtrOffset);
        }
    }

    public struct Names {
        public FireBin.DataReader reader;

        public int namesSign;

        public PtrList names;
        public PtrList extra;

        public int NamesCount => names.Count;
        public int ExtraCount => extra.Count;

        public string GetName(int index) => reader.ReadString(names[index].Value);
        public string GetExtra(int index) => reader.ReadString(extra[index].Value);

    }

    public struct PtrList {
        public FireBin.DataReader reader;
        public FireBin.DataWriter writer;

        public Pointer begin;
        public int Count { get; set; }

        public Pointer? this[int index] {
            get {
                m_CheckIndex(index);
                var offset = begin.offset + index * Pointer.Size;
                return reader.ReadPointer(begin.areaId, offset);
            }
            set {
                m_CheckIndex(index);
                var offset = begin.offset + index * Pointer.Size;
                writer.WritePointer(begin.areaId, value, offset);
            }
        }

        private void m_CheckIndex(int index) {
            if (index < 0 || index >= Count)
                throw new ArgumentOutOfRangeException();
        }
    }

    public struct Scalar {
        public ScalarType scalarType;
        public int valueOffset;

        public object value;
    }

    public struct BinEnum {
        public int intValue;
        public string strValue;
    }

    public struct Pointer {
        public static readonly Pointer NullPointer = new Pointer() { areaId = AreaId.Structs, offset = -1 };
        public static readonly int NullOffset = -1;
        public static readonly int Size = sizeof(byte) + sizeof(int);

        public AreaId areaId;
        public int offset;

        public Pointer TranslateTo(Area area) {
            return new Pointer { areaId = areaId, offset = offset - area.DataOffset };
        }

        public Pointer TranslateFrom(Area area) {
            return new Pointer { areaId = areaId, offset = offset + area.DataOffset };
        }

        public void Check(AreaId areaId) {
            if (this.areaId != areaId)
                throw new FireBinException($"The pointer has areaId: {this.areaId}, but the expected areaId is: {areaId}"); ;
        }

        public override int GetHashCode() {
            return ((long)areaId * 137438953472 + offset).GetHashCode();
        }
    }

    public struct Reference {
        public Pointer from;
        public Pointer to;
        public ulong csRef;
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

        /// Адреса описаний типов.
        private Dictionary<int, int> m_namesOffsets = new Dictionary<int, int>();

        /// Адреса ссылок на ассеты.
        private Dictionary<int, int> m_assetRefOffsets = new Dictionary<int, int>();

        /// Указателей на объекты сохраненные по ссылке. 
        /// Валидны после завершения сериализации.
        private List<Reference> m_references = new List<Reference>();

        /// Адреса указателей.
        /// Валидны сразу.
        private List<Pointer> m_pointers = new List<Pointer>();

        /// Области данных.
        private Area[] m_areas;

        public int ReferencesCount => m_references.Count;
        public int PointersCount => m_pointers.Count;
        public int AreasCount => m_areas.Length;

        public Data() {
            m_areas = new Area[(int)AreaId._Count] {
                new Area(AreaId.Structs),
                new Area(AreaId.Names),
                new Area(AreaId.Scalars),
                new Area(AreaId.AssetRefs),
                new Area(AreaId.Refs),
                new Area(AreaId.Strings),
            };
        }

        public Area this[AreaId index] => m_areas[(int)index];

        public Area this[int index] => m_areas[index];

        public bool HasAssetRefOffset(int assetIdHash) {
            return m_assetRefOffsets.ContainsKey(assetIdHash);
        }

        public int GetAssetRefOffset(int assetIdHash) {
            return m_assetRefOffsets[assetIdHash];
        }

        public void AddAssetRefOffset(int assetIdHash, int offset) {
            m_assetRefOffsets[assetIdHash] = offset;
        }

        public int AddReference(Reference reference) {
            m_references.Add(reference);

            //Console.WriteLine($"ref[{m_references.Count - 1}]: from: ({reference.from.areaId}, {reference.from.offset}) to: ({reference.to.areaId}, {reference.to.offset})");

            return m_references.Count - 1;
        }

        public Reference GetReference(int index) {
            return m_references[index];
        }

        public void SetReference(int index, Reference value) {
            m_references[index] = value;

            //Console.WriteLine($"ref[{index}]: from: ({value.from.areaId}, {value.from.offset}) to: ({value.to.areaId}, {value.to.offset})");

            var dataWriter = new DataWriter(this);
            dataWriter.WriteReferenceData(value.from.offset, value.to, value.csRef);
        }

        public int AddPointer(Pointer ptr) {
            m_pointers.Add(ptr);

            return m_pointers.Count - 1;
        }

        public Pointer GetPointer(int index) {
            return m_pointers[index];
        }

        public void PrintPointers() {
            Console.WriteLine("PrintPointers() >");

            var dataReader = new DataReader(this);

            foreach (var fromPtr in m_pointers) {
                Console.Write($"ptr: from: ({fromPtr.areaId}, {fromPtr.offset})");

                var toPtr = dataReader.ReadPointer(fromPtr.areaId, fromPtr.offset);

                Console.WriteLine($" to: ({toPtr.areaId}, {toPtr.offset})");
            }
            Console.WriteLine("<");
        }

        public void DistinctPointers() {
            m_pointers = m_pointers.Distinct().ToList();
        }

        public void CollectNames() {
            m_namesOffsets.Clear();

            var namesArea = this[AreaId.Names];
            var dataReader = new DataReader(this);

            namesArea.SaveOffset(namesArea.Offset,
                (r, w) => {
                    while (namesArea.Offset < namesArea.Length) {
                        var offset = namesArea.Offset;
                        var names = dataReader.ReadNames();

                        m_namesOffsets[names.namesSign] = offset;
                    }
                });
        }

        public void CollectAssetRefs() {
            m_assetRefOffsets.Clear();

            var assetRefArea = this[AreaId.AssetRefs];
            var dataReader = new DataReader(this);

            assetRefArea.SaveOffset(assetRefArea.Offset,
                (r, w) => {
                    while (assetRefArea.Offset < assetRefArea.Length) {
                        var offset = assetRefArea.Offset;

                        var assetId = dataReader.ReadAssetRef();
                        var assetIdHash = Guid.Parse(assetId).GetHashCode();

                        //Console.WriteLine($"{assetIdHash}: {offset}: {assetId}");

                        m_assetRefOffsets[assetIdHash] = offset;
                    }
                });
        }

        public bool HasNamesOffset(int namesSign) {
            return m_namesOffsets.ContainsKey(namesSign);
        }

        public int GetNamesOffset(int namesSign) {
            return m_namesOffsets[namesSign];
        }

        public void AddNamesOffset(int namesSign, int namesOffset) {
            m_namesOffsets[namesSign] = namesOffset;
        }

        public static int CreateNamesSign(Type type, IEnumerable<string> names, IEnumerable<string> extraNames = null) {
            string str = type.FullName;

            foreach (var name in names)
                str += $"_{name}";

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

        public static BinType ThrowBinType(Type type) {
            var binType = GetBinType(type);
            if (binType == null)
                throw new FireBinException($"Type {type.Name} is an unsupported BinType.");

            return binType.Value;
        }

        public static BinType? GetBinType(Type type) {
            if (type.IsEnum)
                return BinType.Enum;

            if (type == typeof(string))
                return BinType.String;

            if (FireBin.Data.GetScalarType(type) != null)
                return BinType.Scalar;

            if (type == typeof(Engine.Vector2))
                return BinType.Vector2;

            if (type == typeof(Engine.Vector3))
                return BinType.Vector3;

            if (type == typeof(Engine.Quaternion))
                return BinType.Quaternion;

            if (YamlWriter.NeedSaveAsLink(type))
                return BinType.Ref;

            if (YamlWriter.IsList(type))
                return BinType.List;

            if (YamlWriter.IsAsset(type))
                return BinType.AssetRef;

            if (type.IsClass || type.IsValueType)
                return BinType.NamedList;

            return null;
        }

        public static void CheckVector(StructType structType, int size) {
            if (size < 2 || size > 4)
                throw new FireBinException($"Invalid vector size: {size}, support only a 2-4-vector or quaternion.");

            if (structType != StructType.Vector2
                && structType != StructType.Vector3
                && structType != StructType.Quaternion) {
                throw new FireBinException($"Struct type: '{structType}' is not a 2-4-vector or quaternion.");
            }
        }
    }
}
