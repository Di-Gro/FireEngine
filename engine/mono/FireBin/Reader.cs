using System;
using System.IO;
using System.Collections.Generic;
using System.Text;

//using PointerType = System.Int32;

namespace FireBin {

    public class Reader {

        private FireBin.Data m_data;
        private BinaryReader m_reader;

        public Reader() { }

        public FireBin.Data Read(FileStream fileStream) {
            m_data = new Data();
            m_reader = new BinaryReader(fileStream);

            var fbinHeader = new string(m_reader.ReadChars(8));

            for (int i = 0; i < m_data.areas.Length; i++) {
                var area = m_data.areas[i];

                area.Position = m_reader.ReadInt32();
                var length = m_reader.ReadInt32();

                area.writer.Write(m_reader.SaveOffset(area.Position, r => r.ReadBytes(length)));
            }

            var lastArea = m_data.areas[m_data.areas.Length - 1];
            var ptrsPos = lastArea.Position + lastArea.Length;

            ReadReferences(ptrsPos);

            m_data.CollectNames();
            m_data.CollectAssetRefs();


            //Console.WriteLine($"FBIN: {fbinHeader}");
            //Console.WriteLine();
            //Console.WriteLine($"Structs.Position: {m_data[AreaId.Structs].Position} : {m_data[AreaId.Structs].Position.ToString("X")}");
            //Console.WriteLine($"Structs.Length: {m_data[AreaId.Structs].Length} : {m_data[AreaId.Structs].Length.ToString("X")}");
            //Console.WriteLine();
            //Console.WriteLine($"Names.Position: {m_data[AreaId.Names].Position} : {m_data[AreaId.Names].Position.ToString("X")}");
            //Console.WriteLine($"Names.Length: {m_data[AreaId.Names].Length} : {m_data[AreaId.Names].Length.ToString("X")}");
            //Console.WriteLine();
            //Console.WriteLine($"Scalars.Position: {m_data[AreaId.Scalars].Position} : {m_data[AreaId.Scalars].Position.ToString("X")}");
            //Console.WriteLine($"Scalars.Length: {m_data[AreaId.Scalars].Length} : {m_data[AreaId.Scalars].Length.ToString("X")}");
            //Console.WriteLine();
            //Console.WriteLine($"AssetRefs.Position: {m_data[AreaId.AssetRefs].Position} : {m_data[AreaId.AssetRefs].Position.ToString("X")}");
            //Console.WriteLine($"AssetRefs.Length: {m_data[AreaId.AssetRefs].Length} : {m_data[AreaId.AssetRefs].Length.ToString("X")}");
            //Console.WriteLine();
            //Console.WriteLine($"Refs.Position: {m_data[AreaId.Refs].Position} : {m_data[AreaId.Refs].Position.ToString("X")}");
            //Console.WriteLine($"Refs.Length: {m_data[AreaId.Refs].Length} : {m_data[AreaId.Refs].Length.ToString("X")}");
            //Console.WriteLine();
            //Console.WriteLine($"Strings.Position: {m_data[AreaId.Strings].Position} : {m_data[AreaId.Strings].Position.ToString("X")}");
            //Console.WriteLine($"Strings.Length: {m_data[AreaId.Strings].Length} : {m_data[AreaId.Strings].Length.ToString("X")}");
            //Console.WriteLine();


            foreach (var area in m_data.areas)
                area.Position = 0;

            var res = m_data;

            m_data = null;
            m_reader = null;

            return res;
        }

        public void ReadReferences(int position) {
            var length = m_reader.BaseStream.Length;
            var count = (length - position) / Pointer.Size;

            var prevPos = m_reader.BaseStream.Position;
            m_reader.BaseStream.Position = position; 

            for (int i = 0; i < count; i++) {
                var fromPtr = Area.ReadPointerData(m_reader);
                var toPtr = m_reader.SaveOffset(fromPtr.offset, r => Area.ReadPointerData(r));

                var fromArea = m_data[fromPtr.areaId];
                var toArea = m_data[toPtr.areaId];

                fromPtr = fromPtr.TranslateTo(fromArea);
                toPtr = toPtr.TranslateTo(toArea);

                fromArea.writer.SaveOffset(fromPtr.offset, w => Area.WritePointerData(w, toPtr));

                //Console.WriteLine($"pointer[{i}]: {fromAddress.offset} -> {toAddress.offset}");

                m_data.references.Add(new Reference { from = fromPtr, to = toPtr });
            }

            m_reader.BaseStream.Position = prevPos;
        }

        //public void ReadStruct(int position) {
        //    var structType = (StructType)m_reader.ReadByte();
        //    var namesPtr = m_reader.ReadPointer();
        //    var typeGuidPtr = m_reader.ReadPointer();
        //    var basePtr = m_reader.ReadPointer();

        //    Console.WriteLine($"structType: {structType}");
        //    Console.WriteLine($"namesPtr: {namesPtr} : {namesPtr.ToString("X")}");

        //    var names = NamesStruct.Read(m_reader, namesPtr);

        //    Console.WriteLine($"names.count: {names.main.Count}");
        //    Console.WriteLine($"names.extraCount: {names.extra.Count}");

        //    for (int i = 0; i < names.main.Count; i++)
        //        Console.WriteLine($"names.main[{i}]: {names.main[i]}");

        //    for (int i = 0; i < names.extra.Count; i++)
        //        Console.WriteLine($"names.extra[{i}]: {names.extra[i]}");
        //}




        //public List<string> ReadStringsList(int ptr, int count) {
        //    var strings = new List<string>();

        //    var prevPos = m_reader.BaseStream.Position;
        //    m_reader.BaseStream.Position = ptr;

        //    for (int i = 0; i < count; i++) {
        //        var strPtr = m_reader.ReadPointer();
        //        var name = m_reader.Read(strPtr, r => r.ReadString());

        //        strings.Add(name);
        //    }
        //    m_reader.BaseStream.Position = prevPos;

        //    return strings;
        //}

        //private struct NamesStruct {
        //    public List<string> main;
        //    public List<string> extra;

        //    public static NamesStruct Read(BinaryReader reader, int ptr) {
        //        var strct = new NamesStruct();
        //        strct.main = new List<string>();
        //        strct.extra = new List<string>();

        //        var prevPos = reader.BaseStream.Position;
        //        reader.BaseStream.Position = ptr;

        //        var count = reader.ReadInt32();
        //        var extraCount = reader.ReadInt32();

        //        for (int i = 0; i < count + extraCount; i++) {
        //            var strPtr = reader.ReadPointer();
        //            var name = reader.Read(strPtr, r => r.ReadString());

        //            if (i < count)
        //                strct.main.Add(name);
        //            else
        //                strct.extra.Add(name);
        //        }
        //        reader.BaseStream.Position = prevPos;

        //        return strct;
        //    }
        //}

        //private struct EnumStruct {
        //    public int intValue;
        //    public string strValue;

        //    public static EnumStruct Read(BinaryReader reader, int ptr) {
        //        var strct = new EnumStruct();

        //        var prevPos = reader.BaseStream.Position;
        //        reader.BaseStream.Position = ptr;

        //        var structType = reader.ReadByte();
        //        strct.intValue = reader.ReadInt32();
        //        var strValuePtr = reader.ReadPointer();
        //        strct.strValue = reader.Read(strValuePtr, r => r.ReadString());

        //        reader.BaseStream.Position = prevPos;

        //        return strct;
        //    }
        //}

        //public StructType ReadStructType(PointerType structPtr) {

        //}

        //public ValueType GetValueType(PointerType valuePtr) {

        //}

        //public Address? ReadAsNamedList(PointerType structOffset) {
        //    if (obj == null)
        //        return null;

        //    var typeGuid = Engine.GUIDAttribute.GetGuid(type);
        //    var serializer = YamlWriter.GetSerializer(type);
        //    var fields = YamlWriter.GetFields(type, obj, serializer);

        //    var structOffset = m_structs.Offset;
        //    var namesOffset = m_WriteNames(type, fields, serializer);
        //    var namesCount = m_ReadNamesCount(namesOffset);
        //    var extraNamesCount = m_ReadExtraNamesCount(namesOffset);
        //    var guidAddress = WriteAsAssetRef(typeGuid);

        //    m_structs.writer.Write((byte)StructType.NamedList);
        //    m_structs.WritePointer(m_names, namesOffset);
        //    m_structs.WritePointer(guidAddress);

        //    var baseOffset = m_structs.Offset;
        //    m_structs.writer.Write(Pointer.NullPointer);

        //    var fieldsOffset = m_structs.Offset;
        //    for (int i = 0; i < namesCount + extraNamesCount; i++)
        //        m_structs.writer.Write(Pointer.NullPointer);

        //    if (serializer.NeedIncludeBase(type) && type.BaseType != null) {
        //        var address = WriteAsNamedList(type.BaseType, obj);

        //        m_structs.WritePointer(address, baseOffset);
        //    }
        //    for (int i = 0; i < fields.Count; ++i) {
        //        var field = fields[i];
        //        var valueWriter = GetWriter(field.type);
        //        var address = valueWriter.Invoke(field.type, field.Value);
        //        var fieldOffset = fieldsOffset + i * Pointer.Size;

        //        m_structs.WritePointer(address, fieldOffset);
        //    }
        //    if (extraNamesCount > 0) {
        //        var extraFieldsOffset = fieldsOffset + namesCount * Pointer.Size;
        //        m_WriteExtraFields(extraFieldsOffset, extraNamesCount, serializer, type, obj);
        //    }
        //    if (YamlWriter.NeedSaveAsLink(type))
        //        m_AddStructOffset(type, obj, structOffset);

        //    return new Address() { area = m_structs, offset = structOffset };
        //}
    }
}
