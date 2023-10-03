using System;
using System.IO;
using System.Collections.Generic;

using PointerType = System.Int32;

namespace FireBin {
    public class Area {
        public readonly AreaId areaId;

        public MemoryStream stream = new MemoryStream();
        public BinaryWriter writer;
        public BinaryReader reader;

        public PointerType Offset {
            get => (PointerType)stream.Position;
            set => stream.Position = value;
        }

        public PointerType Length => (PointerType)stream.Length;

        public PointerType DataOffset { get; set; }

        public Area(AreaId areaId) {
            this.areaId = areaId;

            writer = new BinaryWriter(stream);
            reader = new BinaryReader(stream);
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

    }
}
