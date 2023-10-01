using System;
using System.IO;
using System.Collections.Generic;
using System.Text;

namespace FireBin {
    public class DataReader {
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

        private FireBin.Data m_data;

        public DataReader(FireBin.Data data) {
            m_data = data;
        }

        public StructType ReadStructType(Pointer structPtr) {
            structPtr.Check(AreaId.Structs);

            var structArea = m_data[AreaId.Structs];

            return structArea.SaveOffset(structPtr.offset, (r, w) => ReadStructType());
        }

        public StructType ReadStructType() {
            var structArea = m_data[AreaId.Structs];

            var structTypeIndex = structArea.reader.ReadByte();

            if (structTypeIndex < 0 || structTypeIndex >= (byte)StructType._Count)
                throw new FireBinException($"Invalid struct type index: {structTypeIndex}");

            return (StructType)structTypeIndex;
        }

        public string ReadScriptId(Pointer namedListPtr) {
            namedListPtr.Check(AreaId.Structs);

            var structArea = m_data[AreaId.Structs];
            return structArea.SaveOffset(namedListPtr.offset, (r, w) => ReadScriptId());
        }

        public string ReadScriptId() {
            var structArea = m_data[AreaId.Structs];

            var structType = ReadStructType();
            if (structType != StructType.NamedList)
                throw new FireBinException($"Struct {structType} don't have a GUID.");

            structArea.Offset += Pointer.Size; // namesPtr

            var scriptIdPtr = ReadPointer(structArea.areaId);
            if (scriptIdPtr.offset == Pointer.NullOffset)
                return "";

            var scriptId = ReadAssetRef(scriptIdPtr);

            return scriptId;
        }

        public string ReadAssetRef(Pointer assetRefPtr) {
            assetRefPtr.Check(AreaId.AssetRefs);

            var assetRefsArea = m_data[AreaId.AssetRefs];

            var assetRef = assetRefsArea.SaveOffset(assetRefPtr.offset, (r, w) => ReadAssetRef());
            return assetRef;
        }
        
        public string ReadAssetRef() {
            var assetRefsArea = m_data[AreaId.AssetRefs];

            var assetRef = assetRefsArea.reader.ReadString();

            return assetRef;
        }

        public Reference ReadReference(Pointer refPtr) {
            refPtr.Check(AreaId.Refs);

            var refsArea = m_data[AreaId.Refs];
            var reference = new Reference() { from = refPtr };

            refsArea.SaveOffset(refPtr.offset, (r, w) => {
                reference.to = ReadPointer(refsArea.areaId);
                reference.csRef = r.ReadUInt64();
            });
            reference.to.Check(AreaId.Structs);

            return reference;
        }

        public NamedList ReadNamedList(Pointer structPtr) {
            structPtr.Check(AreaId.Structs);

            var structArea = m_data[AreaId.Structs];

            return structArea.SaveOffset(structPtr.offset, (r, w) => ReadNamedList());
        }

        public NamedList ReadNamedList() {
            var area = m_data[AreaId.Structs];
            var header = new NamedList();

            header.writer = new DataWriter(m_data);

            header.structOffset = area.Offset;
            header.structType = ReadStructType();
            header.namesPtr = ReadPointer(area.areaId);
            header.scriptIdPtr = ReadPointer(area.areaId);
            header.basePtrOffset = area.Offset;
            header.basePtr = ReadPointer(area.areaId);

            header.names = ReadNames(header.namesPtr);
            header.fields = ReadPtrList(area.areaId, header.names.NamesCount);
            header.extraFields = ReadPtrList(area.areaId, header.names.ExtraCount);

            return header;
        }

        public Names ReadNames(Pointer namesPtr) {
            namesPtr.Check(AreaId.Names);

            var namesArea = m_data[AreaId.Names];

            return namesArea.SaveOffset(namesPtr.offset, (r, w) => ReadNames());
        }

        public Names ReadNames() {
            var areaId = AreaId.Names; 
            var namesArea = m_data[areaId];

            var namesSign = namesArea.reader.ReadInt32();
            var namesCount = namesArea.reader.ReadInt32();
            var extraCount = namesArea.reader.ReadInt32();
            var namesList = ReadPtrList(areaId, namesCount);
            var extraList = ReadPtrList(areaId, extraCount);

            var header = new Names();

            header.reader = this;
            header.namesSign = namesSign;
            header.names = namesList;
            header.extra = extraList;

            return header;
        }

        public PtrList ReadList(Pointer listPtr) {
            listPtr.Check(AreaId.Structs);

            var structArea = m_data[AreaId.Structs];

            return structArea.SaveOffset(listPtr.offset, (r, w) => ReadList());
        }

        public PtrList ReadList() {
            var areaId = AreaId.Structs;
            var structArea = m_data[areaId];

            var structType = ReadStructType();
            if (structType != StructType.List)
                throw new FireBinException($"Struct type: '{structType}' is not a '{StructType.List}'.");

            var count = structArea.reader.ReadInt32();
            var values = ReadPtrList(areaId, count);

            return values;
        }

        public PtrList ReadPtrList(AreaId areaId, int count) {
            var area = m_data[areaId];
            var header = new PtrList();

            header.reader = this;
            header.writer = new DataWriter(m_data);
            header.begin = new Pointer() { areaId = areaId, offset = area.Offset };
            header.Count = count;

            area.Offset += count * Pointer.Size;

            return header;
        }

        public T ReadScalar<T>(Pointer scalarPtr) {
            scalarPtr.Check(AreaId.Scalars);

            var scalarsArea = m_data[AreaId.Scalars];

            var scalar = scalarsArea.SaveOffset(scalarPtr.offset, (r, w) => ReadScalar());

            if (typeof(T) != scalar.value.GetType())
                throw new FireBinException($"Incorrect scalar type: '{scalar.value.GetType()}', when expected type: '{typeof(T)}'.");

            return (T)scalar.value;
        }

        public Scalar ReadScalar(Pointer scalarPtr) {
            scalarPtr.Check(AreaId.Scalars);

            var scalarsArea = m_data[AreaId.Scalars];

            return scalarsArea.SaveOffset(scalarPtr.offset, (r, w) => ReadScalar());
        }

        public Scalar ReadScalar() {
            var scalarsArea = m_data[AreaId.Scalars];

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

        public string ReadString(Pointer stringPtr) {
            stringPtr.Check(AreaId.Strings);

            var stringsArea = m_data[AreaId.Strings];

            return stringsArea.SaveOffset(stringPtr.offset, (r, w) => r.ReadString());
        }

        public float[] ReadVector(Pointer vecPtr, int size) {
            vecPtr.Check(AreaId.Structs);

            var structsArea = m_data[AreaId.Structs];

            return structsArea.SaveOffset(vecPtr.offset, (r, w) => ReadVector(size));
        }

        public float[] ReadVector(int size) {
            var structsArea = m_data[AreaId.Structs];

            var structType = ReadStructType();

            Data.CheckVector(structType, size);

            var values = new float[size];
            for (int i = 0; i < size; i++)
                values[i] = structsArea.reader.ReadSingle();

            return values;
        }

        public BinEnum ReadEnum(Pointer enumPtr) {
            enumPtr.Check(AreaId.Structs);

            var structsArea = m_data[AreaId.Structs];

            return structsArea.SaveOffset(enumPtr.offset, (r, w) => ReadEnum());
        }

        public BinEnum ReadEnum() {
            var structsArea = m_data[AreaId.Structs];

            var structType = ReadStructType();
            if (structType != StructType.Enum)
                throw new FireBinException($"Struct type: '{structType}' is not a '{StructType.Enum}'.");

            var intValue = structsArea.reader.ReadInt32();
            var strValuePtr = ReadPointer(structsArea.areaId);

            strValuePtr.Check(AreaId.Strings);

            var header = new BinEnum();
            header.intValue = intValue;
            header.strValue = ReadString(strValuePtr);

            return header;
        }

        public Pointer ReadPointer(AreaId areaId, int fromOffset = -1) {
            var area = m_data[areaId];

            if (fromOffset < 0)
                return Area.ReadPointerData(area.reader);

            return area.SaveOffset(fromOffset, (r, w) => Area.ReadPointerData(r));
        }

        public BinType ThrowBinType(Pointer ptr) {
            var binType = GetBinType(ptr);
            if (binType == null)
                throw new FireBinException($"Pointer with areaId:{ptr.areaId} to an unsupported BinType.");

            return binType.Value;
        }

        public BinType? GetBinType(Pointer ptr) {
            switch (ptr.areaId) {
                case AreaId.Structs:
                    var structType = ReadStructType(ptr);

                    switch (structType) {
                        case StructType.List: return BinType.List;
                        case StructType.NamedList: return BinType.NamedList;
                        case StructType.Enum: return BinType.Enum;
                        case StructType.Vector2: return BinType.Vector2;
                        case StructType.Vector3: return BinType.Vector3;
                        case StructType.Quaternion: return BinType.Quaternion;
                    }
                    break;
                case AreaId.Scalars: return BinType.Scalar;
                case AreaId.AssetRefs: return BinType.AssetRef;
                case AreaId.Refs: return BinType.Ref;
                case AreaId.Strings: return BinType.String;
            }
            return null;
        }
    }
}
