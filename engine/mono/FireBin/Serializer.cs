using System;
using System.IO;
using System.Collections.Generic;
using System.Collections;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using FireYaml;
using YamlWriter = FireYaml.FireWriter;
using ScalarWriters = System.Collections.Generic.Dictionary<System.Type, System.Action<System.IO.BinaryWriter, object>>;
using ScalarTypes = System.Collections.Generic.Dictionary<FireBin.ScalarType, System.Type>;

using PointerType = System.Int32;

/// TODO: runtime refs (csRefs) для копирования и вставки объектов. 

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

    public enum ValueType {
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

    public struct Pointer {
        public static readonly int NullOffset = -1;
        public static readonly int Size = sizeof(byte) + sizeof(int);

        public AreaId areaId;
        public int offset;

        public Pointer TranslateTo(Area area) {
            return new Pointer { areaId = areaId, offset = offset - area.Position };
        }

        public Pointer TranslateFrom(Area area) {
            return new Pointer { areaId = areaId, offset = offset + area.Position };
        }

        public void Check(AreaId areaId) {
            if (this.areaId != areaId)
                throw new FireBinException($"The pointer has areaId: {this.areaId}, but the expected areaId is: {areaId}"); ;
        }
    }

    public struct Reference {
        public Pointer from;
        public Pointer to; 
    }

    internal struct Link {
        public Type type;
        public object obj;
        public int referenceIndex;
    }

    /// TODO: убрать, есть Pointer
    public class Address {
        public Area area;
        public PointerType offset;
    }

    public class Serializer {
        public delegate Address? AddAsDelegate(Type type, object? obj);

        public static bool showLog = false;

        private Dictionary<int, PointerType> m_objectOffsets = new Dictionary<int, PointerType>();
        
        private List<Link> m_links = new List<Link>();

        private readonly AddAsDelegate[] m_serializers;

        private FireBin.Data m_data;

        public Serializer(FireBin.Data data) {
            m_data = data;

            m_serializers = new AddAsDelegate[(int)ValueType._Count] {
                AddAsList,
                AddAsNamedList,
                AddAsEnum,
                AddAsAssetRef,
                AddAsRef,
                AddAsScalar,
                AddAsString,
                AddAsVector2,
                AddAsVector3,
                AddAsQuaternion,
            };
        }

        public void Serialize() {
            m_ResolveLinks();
        }

        public void Serialize(object obj) {
            AddAsNamedList(obj.GetType(), obj);
            Serialize();
        }

        public Address? AddAsNamedList(Type type, object? obj) {
            if (obj == null)
                return null;

            var structsArea = m_data[AreaId.Structs];
            var namesArea = m_data[AreaId.Names];

            /// - Может ли один объект оказаться здесь несколько раз?
            /// - Только если пользователь запишет его несколько раз.

            var scriptId = Engine.GUIDAttribute.GetGuid(type);
            var serializer = YamlWriter.GetSerializer(type);
            var fields = YamlWriter.GetFields(type, obj, serializer);

            var structOffset = structsArea.Offset;
            var namesOffset = m_data.AddNames(type, fields, serializer);
            var namesCount = m_data.GetNamesCount(namesOffset);
            var extraNamesCount = m_data.GetExtraNamesCount(namesOffset);
            var scriptIdAddress = AddAsAssetRef(scriptId);

            structsArea.writer.Write((byte)StructType.NamedList);
            structsArea.WritePointer(namesArea, namesOffset);
            structsArea.WritePointer(scriptIdAddress);

            var baseOffset = structsArea.Offset;
            structsArea.WritePointer(null);

            /// TODO: кешировать names по сумме имен всех полей, а не только по имени типа.
            if (fields.Count != namesCount)
                throw new FireBinException("fields.Count != namesCount");

            var fieldsOffset = structsArea.Offset;
            for (int i = 0; i < namesCount + extraNamesCount; i++)
                structsArea.WritePointer(null);

            if (serializer.NeedIncludeBase(type) && type.BaseType != null) {
                var address = AddAsNamedList(type.BaseType, obj);

                structsArea.WritePointer(address, baseOffset);
            }
            for (int n = 0; n < fields.Count; ++n) {
                var field = fields[n];
                var valueWriter = GetWriter(field.type);
                var address = valueWriter.Invoke(field.type, field.Value);
                var fieldOffset = fieldsOffset + n * Pointer.Size;

                structsArea.WritePointer(address, fieldOffset);
            }
            if (extraNamesCount > 0) {
                var extraFieldsOffset = fieldsOffset + namesCount * Pointer.Size;
                m_WriteExtraFields(extraFieldsOffset, extraNamesCount, serializer, type, obj);
            }
            if (YamlWriter.NeedSaveAsLink(type))
                m_AddStructOffset(type, obj, structOffset);

            return new Address() { area = structsArea, offset = structOffset };
        }

        public Address? AddAsList(Type type, object? obj) {
            if (obj == null)
                return null;

            var list = obj as IList;
            if (list == null)
                throw new FireBinException("obj is not an IList");

            var structsArea = m_data[AreaId.Structs];

            var structOffset = structsArea.Offset;
            structsArea.writer.Write((byte)StructType.List);
            structsArea.writer.Write(list.Count);

            var itemsOffset = structsArea.Offset;
            for (int i = 0; i < list.Count; i++)
                structsArea.WritePointer(null);

            var genericType = type.GetGenericArguments()[0];
            var valueWriter = GetWriter(genericType);

            for (int n = 0; n < list.Count; ++n) {
                var value = list[n];
                var valueType = value != null ? value.GetType() : genericType;
                var address = valueWriter.Invoke(valueType, value);
                var itemOffset = itemsOffset + n * Pointer.Size;

                structsArea.WritePointer(address, itemOffset);
            }

            return new Address() { area = structsArea, offset = structOffset };
        }

        public Address? AddAsEnum(Type type, object? obj) {
            if (obj == null)
                return null;

            if (!type.IsEnum)
                throw new FireBinException("obj is not an Enum");

            var structsArea = m_data[AreaId.Structs];
            var stringsArea = m_data[AreaId.Strings];

            var intValue = Convert.ToInt32(obj);
            var strValue = obj.ToString();
            var structOffset = structsArea.Offset;
            var stringOffset = stringsArea.Offset;

            stringsArea.writer.Write(strValue);

            structsArea.writer.Write((byte)StructType.Enum);
            structsArea.writer.Write(intValue);
            structsArea.WritePointer(stringsArea, stringOffset);

            return new Address() { area = structsArea, offset = structOffset };
        }

        public Address? AddAsAssetRef(Type type, object? obj) {
            if (obj == null)
                return null;

            var asset = (IAsset)obj;

            return m_data.AddAssetRef(asset.assetId);
        }

        public Address? AddAsAssetRef(string assetId) {
            if (assetId == "")
                return null;

            return m_data.AddAssetRef(assetId);
        }

        public Address? AddAsRef(Type type, object? obj) {
            if (obj == null)
                return null;

            var structsArea = m_data[AreaId.Structs];
            var refsArea = m_data[AreaId.Refs];

            var offset = refsArea.Offset;
            var refIndex = refsArea.WritePointer(structsArea, Pointer.NullOffset);

            m_links.Add(new Link { 
                type = type, 
                obj = obj, 
                referenceIndex = refIndex 
            });

            return new Address() { area = refsArea, offset = offset };
        }
        
        public Address? AddAsScalar(Type type, object? obj) {
            if (obj == null)
                return null;

            var scalarsArea = m_data[AreaId.Scalars];
            var scalarOffset = scalarsArea.Offset;

            m_data.WriteScalar(type, obj);

            return new Address() { area = scalarsArea, offset = scalarOffset };
        }

        public Address? AddAsString(Type type, object? obj) {
            if (obj == null)
                return null;

            var stringsArea = m_data[AreaId.Strings];

            var stringOffset = stringsArea.Offset;

            stringsArea.writer.Write((string)obj);

            return new Address() { area = stringsArea, offset = stringOffset };
        }

        public Address? AddAsVector2(Type type, object? obj) {
            if (obj == null)
                return null;

            var structsArea = m_data[AreaId.Structs];

            var value = (Engine.Vector2)obj;
            var structOffset = structsArea.Offset;

            structsArea.writer.Write((byte)StructType.Vector2);
            structsArea.writer.Write(value.X);
            structsArea.writer.Write(value.Y);

            return new Address() { area = structsArea, offset = structOffset };
        }

        public Address? AddAsVector3(Type type, object? obj) {
            if (obj == null)
                return null;

            var structsArea = m_data[AreaId.Structs];

            var value = (Engine.Vector3)obj;
            var structOffset = structsArea.Offset;

            structsArea.writer.Write((byte)StructType.Vector3);
            structsArea.writer.Write(value.X);
            structsArea.writer.Write(value.Y);
            structsArea.writer.Write(value.Z);

            return new Address() { area = structsArea, offset = structOffset };
        }

        public Address? AddAsQuaternion(Type type, object? obj) {
            if (obj == null)
                return null;

            var structsArea = m_data[AreaId.Structs];

            var value = (Engine.Quaternion)obj;
            var structOffset = structsArea.Offset;

            structsArea.writer.Write((byte)StructType.Quaternion);
            structsArea.writer.Write(value.X);
            structsArea.writer.Write(value.Y);
            structsArea.writer.Write(value.Z);
            structsArea.writer.Write(value.W);

            return new Address() { area = structsArea, offset = structOffset };
        }

        public AddAsDelegate GetWriter(Type type) {
            var valueType = FireBin.Data.GetValueType(type);
            if(valueType == null)
                throw new FireBinException($"Unsupported type: {type.Name}");

            return m_serializers[(int)valueType];
        }

        private void m_WriteExtraFields(PointerType extraFieldsOffset, int extraFieldsCount, Engine.SerializerBase serializer, Type type, object obj) {
            var extraAddresses = serializer.WriteExtraFields(this, type, obj);
            if (extraAddresses.Count != extraFieldsCount)
                throw new FireBinException($"Serializer of type: {type.Name} returned an incorrect number of fields.");

            var structsArea = m_data[AreaId.Structs];

            for (int i = 0; i < extraFieldsCount; ++i) {
                var extraFieldOffset = extraFieldsOffset + i * Pointer.Size;

                structsArea.WritePointer(extraAddresses[i], extraFieldOffset);
            }
        }

        private void m_ResolveLinks() {
            foreach (var link in m_links) {
                var reference = m_data.references[link.referenceIndex];

                reference.to.areaId = AreaId.Structs;
                reference.to.offset = m_GetStructOffset(link.type, link.obj);

                m_data.references[link.referenceIndex] = reference;
            }
        }

        private void m_AddStructOffset(Type type, object obj, PointerType offset) {
            var hash = $"{obj.GetHashCode()}_{type.GetHashCode()}".GetHashCode();

            m_objectOffsets[hash] = offset;
        }

        private PointerType m_GetStructOffset(Type type, object obj) {
            var hash = $"{obj.GetHashCode()}_{type.GetHashCode()}".GetHashCode();

            if (m_objectOffsets.ContainsKey(hash))
                return m_objectOffsets[hash];

            return Pointer.NullOffset;
        }

    }
}
