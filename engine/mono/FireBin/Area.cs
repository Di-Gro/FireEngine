using System;
using System.IO;
using System.Collections.Generic;

using PointerType = System.Int32;

namespace FireBin {
    public class Area {
        public readonly AreaId areaId;

        /// TODO: Shrink(), Extension(), Cut()

        private List<Reference> m_references;

        public MemoryStream stream = new MemoryStream();
        public BinaryWriter writer;
        public BinaryReader reader;

        /// <summary>
        /// Offset from area start.
        /// </summary>
        public PointerType Offset {
            get => (PointerType)stream.Position;
            set => stream.Position = value;
        }

        public PointerType Length => (PointerType)stream.Length;

        /// <summary>
        /// Offset from file start.
        /// </summary>
        public PointerType Position { get; set; }

        public Area(AreaId areaId, List<Reference> references) {
            this.areaId = areaId;
            m_references = references;

            writer = new BinaryWriter(stream);
            reader = new BinaryReader(stream);
        }

        public int WritePointer(Address? toAdress, PointerType fromOffset = -1) {
            if (toAdress != null)
                return WritePointer(toAdress.area, toAdress.offset, fromOffset);

            return WritePointer(null, Pointer.NullOffset, fromOffset);
        }

        public int WritePointer(Area toArea, PointerType toOffset, PointerType fromOffset = -1) {
            bool pushBack = fromOffset < 0;

            var offset = pushBack ? Offset : fromOffset;
            var toAreaId = toArea != null ? toArea.areaId : AreaId.Structs;
            var referenceIndex = m_references.Count;

            m_references.Add(new Reference() {
                from = new Pointer { areaId = this.areaId, offset = offset },
                to = new Pointer { areaId = toAreaId, offset = toOffset }
            });

            if (pushBack)
                WritePointerData(writer, (byte)toAreaId, Pointer.NullOffset);
            else
                SaveOffset(fromOffset, (r, w) => WritePointerData(w, (byte)toAreaId, Pointer.NullOffset));

            return referenceIndex;
        }

        public Pointer ReadPointer(int fromOffset = -1) {
            if (fromOffset < 0)
                return ReadPointerData(reader);

            return SaveOffset(fromOffset, (r, w) => ReadPointerData(r));
        }

        public static void WritePointerData(BinaryWriter w, Pointer pointer) {
            WritePointerData(w, (byte)pointer.areaId, pointer.offset);
        }

        public static void WritePointerData(BinaryWriter w, byte toAreaId, int toOffset) {
            w.Write(toAreaId);
            w.Write(toOffset);
        }

        public static Pointer ReadPointerData(BinaryReader r) {
            var ptr = new Pointer() { };

            var areaIdIndex = r.ReadByte();
            if (areaIdIndex < 0 || areaIdIndex >= (byte)AreaId._Count)
                throw new FireBinException($"Invalid pointer (areaId:{areaIdIndex}).");

            ptr.areaId = (AreaId)areaIdIndex;
            ptr.offset = r.ReadInt32();

            return ptr;
        }

        public T SaveOffset<T>(int offset, Func<BinaryReader, BinaryWriter, T> action) {
            var prevPos = Offset;

            Offset = offset;
            var res = action(reader, writer);
            Offset = prevPos;

            return res;
        }

        public void SaveOffset(int offset, Action<BinaryReader, BinaryWriter> action) {
            var prevPos = Offset;

            Offset = offset;
            action(reader, writer);
            Offset = prevPos;
        }

    }
}
