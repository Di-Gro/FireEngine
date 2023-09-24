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

namespace FireBin {

    internal enum StructType {
        List,
        NamedList,
        Enum,
    }

    internal enum WriteAs {
        List,
        NamedList,
        Enum,
        AssetRef,
        Ref,
        Scalar,
        String,
        _Count,
    }

    public struct Pointer {
        public static readonly long NullPointer = -1;
        public static readonly int PointerLength = sizeof(long);

        public Area fromArea;
        public long fromOffset;

        public Area toArea;
        public long toOffset;
    }

    internal struct Link {
        public Type type;
        public object obj;
        public int pointerIndex;
    }

    public class Address {
        public Area area;
        public long offset;
    }

    public class Writer {
        public delegate Address? WriteAsDelegate(Type type, object? obj);

        public static bool showLog = false;

        private static readonly ScalarWriters s_scalarWriters = new ScalarWriters {
            { typeof(bool), (writer, obj) => writer.Write((bool)obj) },
            { typeof(byte), (writer, obj) => writer.Write((byte)obj) },
            { typeof(char), (writer, obj) => writer.Write((char)obj) },
            { typeof(decimal), (writer, obj) => writer.Write((decimal)obj) },
            { typeof(double), (writer, obj) => writer.Write((double)obj) },
            { typeof(short), (writer, obj) => writer.Write((short)obj) },
            { typeof(int), (writer, obj) => writer.Write((int)obj) },
            { typeof(long), (writer, obj) => writer.Write((long)obj) },
            { typeof(sbyte), (writer, obj) => writer.Write((sbyte)obj) },
            { typeof(float), (writer, obj) => writer.Write((float)obj) },
            { typeof(ushort), (writer, obj) => writer.Write((ushort)obj) },
            { typeof(uint), (writer, obj) => writer.Write((uint)obj) },
            { typeof(ulong), (writer, obj) => writer.Write((ulong)obj) },
        };

        private Dictionary<int, long> m_objectOffsets = new Dictionary<int, long>();
        private Dictionary<int, long> m_namesOffsets = new Dictionary<int, long>();
        private List<Pointer> m_pointers = new List<Pointer>();
        private List<Link> m_links = new List<Link>();

        private readonly WriteAsDelegate[] m_writers;

        private Area m_structs;
        private Area m_names;
        private Area m_scalars;
        private Area m_assetRefs;
        private Area m_refs;
        private Area m_strings;

        public Writer() {
            m_structs = new Area(m_pointers);
            m_names = new Area(m_pointers);
            m_scalars = new Area(m_pointers);
            m_assetRefs = new Area(m_pointers);
            m_refs = new Area(m_pointers);
            m_strings = new Area(m_pointers);

            m_writers = new WriteAsDelegate[(int)WriteAs._Count] {
                WriteAsList,
                WriteAsNamedList,
                WriteAsEnum,
                WriteAsAssetRef,
                WriteAsRef,
                WriteAsScalar,
                WriteAsString,
            };
        }

        public void Write(BinaryWriter writer) {
            var areas = new Area[] {
                m_structs,
                m_names,
                m_scalars,
                m_assetRefs,
                m_refs,
                m_strings,
            };

            foreach (var area in areas) {
                area.Position = writer.BaseStream.Position;
                writer.Write(area.stream.ToArray());
            }

            m_ResolvePointers(writer);
        }

        public Address? WriteAsList(Type type, object? obj) {
            if (obj == null)
                return null;

            var list = obj as IList;
            if (list == null)
                throw new ArgumentException("obj is not an IList");

            var structOffset = m_structs.Offset;
            m_structs.writer.Write((byte)StructType.List);
            m_structs.writer.Write(list.Count);

            var itemsOffset = m_structs.Offset;
            for (int i = 0; i < list.Count; i++)
                m_structs.writer.Write(Pointer.NullPointer);

            var genericType = type.GetGenericArguments()[0];
            var valueWriter = GetWriter(genericType);

            for (int i = 0; i < list.Count; ++i) {
                var value = list[i];
                var valueType = value != null ? value.GetType() : genericType;
                var address = valueWriter.Invoke(valueType, value);
                var itemOffset = itemsOffset + i * sizeof(long);

                m_structs.WritePointer(address, itemOffset);
            }

            return new Address() { area = m_structs, offset = structOffset };
        }

        public Address? WriteAsNamedList(Type type, object? obj) {
            if (obj == null)
                return null;

            /// - Может ли один объект оказаться здесь несколько раз?
            /// - Только если пользователь запишет его несколько раз.

            var typeGuid = Engine.GUIDAttribute.GetGuid(type);
            var serializer = YamlWriter.GetSerializer(type);
            var fields = YamlWriter.GetFields(type, obj, serializer);

            var structOffset = m_structs.Offset;
            var namesOffset = m_WriteNames(type, fields, serializer);
            var namesCount = m_ReadNamesCount(namesOffset);
            var extraNamesCount = m_ReadExtraNamesCount(namesOffset);
            var guidAddress = WriteAsAssetRef(typeGuid);
                        
            m_structs.writer.Write((byte)StructType.NamedList);
            m_structs.WritePointer(m_names, namesOffset);
            m_structs.WritePointer(guidAddress);

            var baseOffset = m_structs.Offset;
            m_structs.writer.Write(Pointer.NullPointer);

            var fieldsOffset = m_structs.Offset;
            for (int i = 0; i < namesCount + extraNamesCount; i++)
                m_structs.writer.Write(Pointer.NullPointer);

            if (serializer.NeedIncludeBase(type) && type.BaseType != null) {
                var address = WriteAsNamedList(type.BaseType, obj);

                m_structs.WritePointer(address, baseOffset);
            }

            for (int i = 0; i < fields.Count; ++i) {
                var field = fields[i];
                var valueWriter = GetWriter(field.type);
                var address = valueWriter.Invoke(field.type, field.Value);
                var fieldOffset = fieldsOffset + i * Pointer.PointerLength;

                m_structs.WritePointer(address, fieldOffset);
            }

            if (YamlWriter.NeedSaveAsLink(type))
                m_AddStructOffset(type, obj, structOffset);

            /// TODO: Вызвать пользовательскую сериализацию
            //serializer.OnSerialize(this, selfPath, type, obj);

            return new Address() { area = m_structs, offset = structOffset };
        }

        public Address? WriteAsEnum(Type type, object? obj) {
            if (obj == null)
                return null;

            if (!type.IsEnum)
                throw new ArgumentException("obj is not an Enum");

            var intValue = Convert.ToInt32(obj);
            var strValue = obj.ToString();
            var structOffset = m_structs.Offset;
            var stringOffset = m_strings.Offset;

            m_strings.writer.Write(strValue);

            m_structs.writer.Write((byte)StructType.Enum);
            m_structs.writer.Write(intValue);
            m_structs.WritePointer(m_strings, stringOffset);

            return new Address() { area = m_structs, offset = structOffset };
        }

        public Address? WriteAsAssetRef(Type type, object? obj) {
            if (obj == null)
                return null;

            var asset = (IAsset)obj;
            var assetId = asset.assetId;

            var assetRefOffset = m_assetRefs.Offset;

            m_assetRefs.writer.Write(assetId);

            return new Address() { area = m_assetRefs, offset = assetRefOffset };
        }

        public Address? WriteAsAssetRef(string assetId) {
            if (assetId == "")
                return null;

            var assetRefOffset = m_assetRefs.Offset;

            m_assetRefs.writer.Write(assetId);

            return new Address() { area = m_assetRefs, offset = assetRefOffset };
        }

        public Address? WriteAsRef(Type type, object? obj) {
            if (obj == null)
                return null;

            var refOffset = m_refs.Offset;
            var linkIndex = m_pointers.Count;

            m_pointers.Add(new Pointer {
                fromArea = m_refs,
                fromOffset = refOffset,
                toArea = m_structs,
                toOffset = Pointer.NullPointer,
            });

            m_refs.writer.Write(Pointer.NullPointer);

            m_links.Add(new Link { 
                type = type, 
                obj = obj, 
                pointerIndex = linkIndex 
            });

            return new Address() { area = m_refs, offset = refOffset };
        }
        
        public Address? WriteAsScalar(Type type, object? obj) {
            if (obj == null)
                return null;

            var scalarOffset = m_scalars.Offset;

            if (s_scalarWriters.ContainsKey(type))
                s_scalarWriters[type].Invoke(m_scalars.writer, obj);
            else
                throw new ArgumentException($"type is not a scalar type: {type}");

            return new Address() { area = m_scalars, offset = scalarOffset };
        }

        public Address? WriteAsString(Type type, object? obj) {
            if (obj == null)
                return null;

            var stringOffset = m_strings.Offset;

            m_strings.writer.Write((string)obj);

            return new Address() { area = m_strings, offset = stringOffset };
        }

        public WriteAsDelegate GetWriter(Type type) {
            if (type.IsEnum)
                return m_writers[(int)WriteAs.Enum];

            if (type == typeof(string))
                return m_writers[(int)WriteAs.String];

            if (s_scalarWriters.ContainsKey(type))
                return m_writers[(int)WriteAs.Scalar];

            if (YamlWriter.NeedSaveAsLink(type))
                return m_writers[(int)WriteAs.Ref];

            if (YamlWriter.IsList(type))
                return m_writers[(int)WriteAs.List];

            if (YamlWriter.IsAsset(type))
                return m_writers[(int)WriteAs.AssetRef];

            if (type.IsClass || type.IsValueType)
                return m_writers[(int)WriteAs.NamedList];

            throw new ArgumentException($"Unsupported type: {type.Name}");
        }

        private void m_ResolvePointers(BinaryWriter writer) {
            var lastPos = writer.BaseStream.Position;

            foreach (var link in m_links) {
                var pointer = m_pointers[link.pointerIndex];

                pointer.toOffset = m_GetStructOffset(link.type, link.obj);

                m_pointers[link.pointerIndex] = pointer;
            }
            foreach (var pointer in m_pointers) {
                if (pointer.toOffset == Pointer.NullPointer)
                    continue;

                var fromOffset = pointer.fromArea.Position + pointer.fromOffset;
                var toOffset = pointer.toArea.Position + pointer.toOffset;

                writer.BaseStream.Position = fromOffset;
                writer.Write(toOffset);
            }
            writer.BaseStream.Position = lastPos;
        }

        private int m_ReadNamesCount(long namesOffset) {
            var lastOffset = m_names.Offset;

            m_names.Offset = namesOffset;
            var count = m_names.reader.ReadInt32();

            m_names.Offset = lastOffset;

            return count;
        }

        private int m_ReadExtraNamesCount(long namesOffset) {
            var lastOffset = m_names.Offset;

            m_names.Offset = namesOffset + sizeof(int);
            var count = m_names.reader.ReadInt32();

            m_names.Offset = lastOffset;

            return count;
        }

        private long m_WriteNames(Type type, List<Field> fields, Engine.SerializerBase serializer) {
            var typeId = type.FullName.GetHashCode();

            if (m_namesOffsets.ContainsKey(typeId))
                return m_namesOffsets[typeId];

            var extraNames = serializer.GetNamesOfExtraFields();
            var namesOffset = m_names.Offset;

            m_namesOffsets[typeId] = namesOffset;
            m_names.writer.Write(fields.Count);
            m_names.writer.Write(extraNames != null ? extraNames.Count : 0);

            foreach (var field in fields) {
                m_names.WritePointer(m_strings, m_strings.Offset);
                m_strings.writer.Write(field.name);
            }
            if(extraNames != null) {
                foreach (var name in extraNames) {
                    m_names.WritePointer(m_strings, m_strings.Offset);
                    m_strings.writer.Write(name);
                }
            }
            return namesOffset;
        }

        //private long m_WriteNames(Type type) {
        //    var typeId = type.FullName.GetHashCode();

        //    if (m_namesOffsets.ContainsKey(typeId))
        //        return m_namesOffsets[typeId];

        //    var names = GetNames(type);
        //    var namesOffset = m_names.Offset;

        //    m_namesOffsets[typeId] = namesOffset;
        //    m_names.writer.Write(names.Count);

        //    foreach (var name in names) {
        //        m_names.WriteLink(m_strings, m_strings.Offset);
        //        m_strings.writer.Write(name);
        //    }
        //    return namesOffset;
        //}

        private void m_AddStructOffset(Type type, object obj, long offset) {
            var hash = $"{obj.GetHashCode()}_{type.GetHashCode()}".GetHashCode();

            m_objectOffsets[hash] = offset;
        }

        private long m_GetStructOffset(Type type, object obj) {
            var hash = $"{obj.GetHashCode()}_{type.GetHashCode()}".GetHashCode();

            if (m_objectOffsets.ContainsKey(hash))
                return m_objectOffsets[hash];

            return Pointer.NullPointer;
        }

        //public static List<string> GetNames(Type type) {
        //    var names = new List<string>();

        //    var serializer = YamlWriter.GetSerializer(type);

        //    if (type.IsEnum)
        //        return names;

        //    var allFields = type.GetFields(YamlWriter.s_flags);
        //    var allProps = type.GetProperties(YamlWriter.s_flags);

        //    foreach (var field in allFields) {
        //        if (field.DeclaringType != type || !YamlWriter.CanSerialize(field.FieldType))
        //            continue;
        //        if (serializer.NeedSerialize(field)) {
        //            if (showLog) Console.WriteLine($"#: GetNames: field:{field.Name}");
        //            names.Add(field.Name);
        //        }
        //    }
        //    foreach (var prop in allProps) {
        //        if (prop.DeclaringType != type || !YamlWriter.CanSerialize(prop.PropertyType))
        //            continue;
        //        if (serializer.NeedSerialize(prop)) {
        //            if (showLog) Console.WriteLine($"#: GetNames: prop:{prop.Name}");
        //            names.Add(prop.Name);
        //        }
        //    }
        //    return names;
        //}


    }
}
