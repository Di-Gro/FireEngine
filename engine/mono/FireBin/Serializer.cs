using System;
using System.IO;
using System.Collections.Generic;
using System.Collections;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using FireYaml;
using YamlWriter = FireYaml.FireWriter;

/// TODO: runtime refs (csRefs) для копирования и вставки объектов. 

namespace FireBin {

    internal struct Link {
        public Type type;
        public object obj;
        public int referenceIndex;
    }

    public class Serializer {
        public delegate Pointer? AddAsDelegate(Type type, object? obj);

        public static bool showLog = false;

        private Dictionary<int, int> m_objectOffsets = new Dictionary<int, int>();
        
        private List<Link> m_links = new List<Link>();

        private readonly AddAsDelegate[] m_serializers;

        private FireBin.Data m_data;
        private FireBin.DataWriter m_writer;
        private FireBin.DataReader m_reader;

        public Serializer(FireBin.Data data) {
            m_data = data;
            m_writer = new DataWriter(data);
            m_reader = new DataReader(data);

            m_serializers = new AddAsDelegate[(int)BinType._Count] {
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

        public void Serialize(object obj) {
            AddAsNamedList(obj.GetType(), obj);
            Serialize();
        }

        //public Serializer Add(object obj) {
        //    AddAsNamedList(obj.GetType(), obj);

        //    return this;
        //}

        public void Serialize() {
            m_ResolveLinks();
            //m_data.PrintPointers();
        }

        public Pointer? AddAsNamedList(Type type) {
            var scriptId = Engine.GUIDAttribute.GetGuid(type);
            if (scriptId == "")
                throw new FireBinException($"BinType: {BinType.NamedList} must have a scriptId.");

            var namedListPtr = m_writer.WriteNamedList(type, scriptId, new string[0], null);
           
            return namedListPtr;
        }

        public Pointer? AddAsNamedList(Type type, object? obj) {
            if (obj == null)
                return null;

            /// ASK: Может ли один объект оказаться здесь несколько раз?
            /// ---: Да, если пользователь запишет его несколько раз.
            /// TODO: Решить что делать в таком случае.

            var scriptId = Engine.GUIDAttribute.GetGuid(type);
            if (scriptId == "")
                throw new FireBinException($"BinType: {BinType.NamedList} must have a scriptId.");

            var serializer = YamlWriter.GetSerializer(type);
            var fields = YamlWriter.GetFields(type, obj, serializer);
            var names = fields.Select(f => f.name);
            var extraNames = serializer.GetNamesOfExtraFields();

            var namedListPtr = m_writer.WriteNamedList(type, scriptId, names, extraNames);
            var namedList = m_reader.ReadNamedList(namedListPtr);

            if (fields.Count != namedList.fields.Count)
                throw new FireBinException("fields.Count != namesCount");

            if (serializer.NeedIncludeBase(type) && type.BaseType != null)
                namedList.WriteBasePtr(AddAsNamedList(type.BaseType, obj));

            for (int n = 0; n < fields.Count; ++n) {
                var field = fields[n];

                //Console.WriteLine($"field: {field.name}");

                var valueWriter = GetWriter(field.type);
                var valuePtr = valueWriter.Invoke(field.type, field.Value);

                namedList.fields[n] = valuePtr;
            }
            if (namedList.extraFields.Count > 0) {
                var extraPointers = serializer.WriteExtraFields(this, type, obj);
                if (extraPointers.Count != namedList.extraFields.Count)
                    throw new FireBinException($"Serializer of type: {type.Name} returned an incorrect number of fields.");

                for (int i = 0; i < namedList.extraFields.Count; ++i)
                    namedList.extraFields[i] = extraPointers[i];
            }
            if (YamlWriter.NeedSaveAsLink(type))
                m_AddStructOffset(type, obj, namedList.structOffset);

            return namedListPtr;
        }

        public Pointer? AddAsList(Type type, object? obj) {
            if (obj == null)
                return null;

            var listObj = obj as IList;
            if (listObj == null)
                throw new FireBinException("obj is not an IList");

            var listPtr = m_writer.WriteList(listObj.Count);
            var list = m_reader.ReadList(listPtr);

            var genericType = type.GetGenericArguments()[0];
            var valueWriter = GetWriter(genericType);

            for (int n = 0; n < listObj.Count; ++n) {
                var value = listObj[n];
                var valueType = value != null ? value.GetType() : genericType;

                // Console.WriteLine($"item: {n}");

                list[n] = valueWriter.Invoke(valueType, value);
            }
            return listPtr;
        }

        public Pointer? AddAsEnum(Type type, object? obj) {
            if (obj == null)
                return null;

            if (!type.IsEnum)
                throw new FireBinException("obj is not an Enum");

            return m_writer.WriteEnum(obj);
        }

        public Pointer? AddAsAssetRef(Type type, object? obj) {
            if (obj == null)
                return null;

            var asset = (IAsset)obj;

            return m_writer.WriteAssetRef(asset.assetId);
        }

        public Pointer? AddAsAssetRef(string assetId) {
            if (assetId == "")
                return null;

            return m_writer.WriteAssetRef(assetId);
        }

        public Pointer? AddAsRef(Type type, object? obj) {
            if (obj == null)
                return null;

            ulong csRef = Engine.CppLinked.NullRef;

            var cppLinked = obj as Engine.CppLinked;
            if (cppLinked != null)
                csRef = cppLinked.csRef.value;

            int refIndex;
            var refPtr = m_writer.WriteReference(Pointer.NullPointer, csRef, out refIndex);

            //Console.WriteLine($"ref.index: {refIndex}");

            m_links.Add(new Link { 
                type = type, 
                obj = obj, 
                referenceIndex = refIndex 
            });
            return refPtr;
        }
        
        public Pointer? AddAsScalar(Type type, object? obj) {
            if (obj == null)
                return null;

            return m_writer.WriteScalar(type, obj);
        }

        public Pointer? AddAsString(Type type, object? obj) {
            if (obj == null)
                return null;

            return m_writer.WriteString((string)obj);
        }

        public Pointer? AddAsVector2(Type type, object? obj) {
            if (obj == null)
                return null;

            var value = (Engine.Vector2)obj;

            return m_writer.WriteVector(StructType.Vector2, new float[] { value.X, value.Y });
        }

        public Pointer? AddAsVector3(Type type, object? obj) {
            if (obj == null)
                return null;

            var value = (Engine.Vector3)obj;

            return m_writer.WriteVector(StructType.Vector3, new float[] { value.X, value.Y, value.Z });
        }

        public Pointer? AddAsQuaternion(Type type, object? obj) {
            if (obj == null)
                return null;

            var value = (Engine.Quaternion)obj;

            return m_writer.WriteVector(StructType.Quaternion, new float[] { value.X, value.Y, value.Z, value.W });
        }

        public AddAsDelegate GetWriter(Type type) {
            var valueType = FireBin.Data.ThrowBinType(type);
            return m_serializers[(int)valueType];
        }

        private void m_ResolveLinks() {
            foreach (var link in m_links) {
                var reference = m_data.GetReference(link.referenceIndex);

                reference.to.areaId = AreaId.Structs;
                reference.to.offset = m_GetStructOffset(link.type, link.obj);

                //Console.WriteLine($"ref.index: {link.referenceIndex} to offset: {reference.to.offset}");

                m_data.SetReference(link.referenceIndex, reference);
            }
        }

        private void m_AddStructOffset(Type type, object obj, int offset) {
            var hash = $"{obj.GetHashCode()}_{type.GetHashCode()}".GetHashCode();

            m_objectOffsets[hash] = offset;
        }

        private int m_GetStructOffset(Type type, object obj) {
            var hash = $"{obj.GetHashCode()}_{type.GetHashCode()}".GetHashCode();

            if (m_objectOffsets.ContainsKey(hash))
                return m_objectOffsets[hash];

            return Pointer.NullOffset;
        }

    }
}
