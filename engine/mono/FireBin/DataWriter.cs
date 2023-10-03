using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;

namespace FireBin {
    public class DataWriter {
        
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

        private FireBin.Data m_data;

        public DataWriter(FireBin.Data data) {
            m_data = data;
        }

        public void Write(BinaryWriter writer) {
            var dataPosition = writer.BaseStream.Position;

            writer.Write("FBINv001".ToArray());

            var areaInfoOffsets = writer.BaseStream.Position;
            for (int i = 0; i < m_data.AreasCount; i++) {
                writer.Write(Pointer.NullOffset);
                writer.Write(Pointer.NullOffset);
            }

            for (int i = 0; i < m_data.AreasCount; i++) {
                var area = m_data[i];
                var areaInfoOffset = areaInfoOffsets + i * sizeof(int) * 2;
                var areaLength = (int)area.writer.BaseStream.Length;

                area.DataOffset = (int)writer.BaseStream.Position - (int)dataPosition;
                writer.Write(area.stream.ToArray());

                writer.SaveOffset(areaInfoOffset, w => {
                    w.Write(area.DataOffset);
                    w.Write(areaLength);
                });
            }

            m_WritePointers(writer);

            for (int i = 0; i < m_data.AreasCount; i++)
                m_data[i].DataOffset = 0;
        }

        private void m_WritePointers(BinaryWriter writer) {
            int writedRefsCount = 0;

            m_data.DistinctPointers();

            for (int i = 0; i < m_data.PointersCount; ++i) {
                var fromPtr = m_data.GetPointer(i);
                if (fromPtr.offset == Pointer.NullOffset)
                    continue;

                Area.WritePointerData(writer, fromPtr);

                writedRefsCount++;
            }

            //Console.WriteLine($"Writed Refs Count: {writedRefsCount}");
        }

        public Pointer WriteReference(Pointer toPtr, ulong csRef, out int refIndex) {
            var refsArea = m_data[AreaId.Refs];
            
            var refOffset = refsArea.Offset;
            WriteReferenceData(toPtr, csRef);

            var fromPtr = new Pointer() { areaId = refsArea.areaId, offset = refOffset };

            refIndex = m_data.AddReference(new Reference() { from = fromPtr, to = toPtr, csRef = csRef });

            return fromPtr;
        }

        public void WriteReferenceData(int fromOffset, Pointer toPtr, ulong csRef) {
            var refsArea = m_data[AreaId.Refs];

            refsArea.SaveOffset(fromOffset, (r, w) => WriteReferenceData(toPtr, csRef));
        }

        public void WriteReferenceData(Pointer toPtr, ulong csRef) {
            var refsArea = m_data[AreaId.Refs];

            WritePointer(refsArea.areaId, toPtr);
            refsArea.writer.Write(csRef);
        }

        public void WriteStructType(StructType structType) {
            var structsArea = m_data[AreaId.Structs];

            structsArea.writer.Write((byte)structType);
        }

        public Pointer WriteNamedList(Type type, string scriptId, IEnumerable<string> fields, IEnumerable<string> extraFields = null) {
            var areaId = AreaId.Structs;
            var structsArea = m_data[areaId];
            int totalCount;

            var structOffset = structsArea.Offset;
            var namesPtr = WriteNames(out totalCount, type, fields, extraFields);
            var scriptIdPtr = WriteAssetRef(scriptId);

            WriteStructType(StructType.NamedList);
            WritePointer(areaId, namesPtr);
            WritePointer(areaId, scriptIdPtr);
            WritePointer(areaId, null);

            for (int i = 0; i < totalCount; i++)
                WritePointer(areaId, null);

            return new Pointer() { areaId = areaId, offset = structOffset };
        }

        public Pointer WriteNames(out int totalCount, Type type, IEnumerable<string> names, IEnumerable<string> extraNames = null) {
            var namesSign = Data.CreateNamesSign(type, names, extraNames);
            int namesCount = 0;
            int extraCount = 0; 
            totalCount = 0;

            if (m_data.HasNamesOffset(namesSign)) {
                foreach (var name in names) 
                    namesCount++;
                    
                if (extraNames != null)
                    foreach (var name in extraNames) 
                        extraCount++;

                totalCount = namesCount + extraCount;
                return new Pointer { areaId = AreaId.Names, offset = m_data.GetNamesOffset(namesSign) };
            }
            var namesArea = m_data[AreaId.Names];
            var stringsArea = m_data[AreaId.Strings];
            var namesOffset = namesArea.Offset;

            m_data.AddNamesOffset(namesSign, namesOffset);
            namesArea.writer.Write(namesSign);

            int countOffset = namesArea.Offset;
            namesArea.writer.Write(namesCount);
            namesArea.writer.Write(extraCount);

            foreach (var name in names) {
                WritePointer(namesArea.areaId, stringsArea.areaId, stringsArea.Offset);
                stringsArea.writer.Write(name);
                namesCount++;
            }
            if (extraNames != null) {
                foreach (var name in extraNames) {
                    WritePointer(namesArea.areaId, stringsArea.areaId, stringsArea.Offset);
                    stringsArea.writer.Write(name);
                    extraCount++;
                }
            }
            namesArea.SaveOffset(countOffset,
                (r, w) => {
                    w.Write(namesCount);
                    w.Write(extraCount);
                });

            totalCount = namesCount + extraCount;
            return new Pointer { areaId = AreaId.Names, offset = namesOffset };
        }

        public Pointer WriteList(int itemsCount) {
            var structsArea = m_data[AreaId.Structs];

            var structOffset = structsArea.Offset;
            WriteStructType(StructType.List);
            structsArea.writer.Write(itemsCount);

            for (int i = 0; i < itemsCount; i++)
                WritePointer(structsArea.areaId, null);

            return new Pointer() { areaId = AreaId.Structs, offset = structOffset };
        }

        public Pointer WriteEnum(object enumObj) {
            var structsArea = m_data[AreaId.Structs];
            var stringsArea = m_data[AreaId.Strings];

            var intValue = Convert.ToInt32(enumObj);
            var strValue = enumObj.ToString();
            var structOffset = structsArea.Offset;
            var stringOffset = stringsArea.Offset;

            stringsArea.writer.Write(strValue);

            WriteStructType(StructType.Enum);
            structsArea.writer.Write(intValue);
            WritePointer(structsArea.areaId, stringsArea.areaId, stringOffset);

            return new Pointer() { areaId = AreaId.Structs, offset = structOffset };
        }

        public Pointer WriteAssetRef(string assetId) {
            var assetRefArea = m_data[AreaId.AssetRefs];
            int assetIdHash;

            Guid guid;
            if (Guid.TryParse(assetId, out guid))
                assetIdHash = guid.GetHashCode();
            else
                assetIdHash = assetId.GetHashCode();

            if (m_data.HasAssetRefOffset(assetIdHash))
                return new Pointer() { areaId = assetRefArea.areaId, offset = m_data.GetAssetRefOffset(assetIdHash) };

            var offset = assetRefArea.Offset;

            m_data.AddAssetRefOffset(assetIdHash, offset);

            assetRefArea.writer.Write(assetId);

            return new Pointer() { areaId = assetRefArea.areaId, offset = offset };
        }

        public Pointer WriteScalar(Type type, object obj) {
            var scalarType = Data.GetScalarType(type);
            if (scalarType == null)
                throw new FireBinException($"Type: {type.Name} is not a scalar type.");

            var scalarsArea = m_data[AreaId.Scalars];
            var scalarWriter = s_scalarWriters[(int)scalarType];
            var scalarOffset = scalarsArea.Offset;

            scalarsArea.writer.Write((byte)scalarType);
            scalarWriter.Invoke(scalarsArea.writer, obj);

            return new Pointer() { areaId = scalarsArea.areaId, offset = scalarOffset };
        }

        public Pointer WriteString(string value) {
            var stringsArea = m_data[AreaId.Strings];
            var stringOffset = stringsArea.Offset;

            stringsArea.writer.Write(value);

            return new Pointer() { areaId = AreaId.Strings, offset = stringOffset };
        }

        public Pointer WriteVector(StructType structType, float[] fields) {
            Data.CheckVector(structType, fields.Length);

            var structsArea = m_data[AreaId.Structs];
            var structOffset = structsArea.Offset;

            WriteStructType(structType);
            foreach (var field in fields)
                structsArea.writer.Write(field);

            return new Pointer() { areaId = structsArea.areaId, offset = structOffset };
        }

        public void WritePointer(AreaId fromAreaId, Pointer? toPtr, int fromOffset = -1) {
            if (toPtr != null)
                WritePointer(fromAreaId, toPtr.Value.areaId, toPtr.Value.offset, fromOffset);
            else
                WritePointer(fromAreaId, AreaId.Structs, Pointer.NullOffset, fromOffset);
        }

        public void WritePointer(AreaId fromAreaId, AreaId toAreaId, int toOffset, int fromOffset = -1) {
            var area = m_data[fromAreaId];

            bool pushBack = fromOffset < 0;
            var offset = pushBack ? area.Offset : fromOffset;

            var fromPtr = new Pointer { areaId = fromAreaId, offset = offset };
            var toPtr = new Pointer { areaId = toAreaId, offset = toOffset };

            if (pushBack)
                Area.WritePointerData(area.writer, (byte)toPtr.areaId, toPtr.offset);
            else
                area.SaveOffset(fromOffset, (r, w) => Area.WritePointerData(w, (byte)toPtr.areaId, toPtr.offset));

            m_data.AddPointer(fromPtr);

            //Console.WriteLine($"ptr: from:({fromPtr.areaId}, {fromPtr.offset}) to:({toPtr.areaId}, {toPtr.offset})");
        }
    }
}
