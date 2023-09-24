using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace FireBin {
    public class Area {
        private List<Pointer> m_pointers;

        public MemoryStream stream = new MemoryStream();
        public BinaryWriter writer;
        public BinaryReader reader;

        /// <summary>
        /// Offset from area start.
        /// </summary>
        public long Offset {
            get => stream.Position;
            set => stream.Position = value;
        }
        
        /// <summary>
        /// Offset from file start.
        /// </summary>
        public long Position { get; set; }

        public Area(List<Pointer> links) {
            m_pointers = links;

            writer = new BinaryWriter(stream);
            reader = new BinaryReader(stream);
        }

        public void Write(long offset, Action<BinaryWriter> writeAction) {
            long prevOffset = Offset;

            Offset = offset;
            writeAction(writer);
            Offset = prevOffset;
        }


        public void WritePointer(Address? toAdress, long fromOffset = -1) {
            if (toAdress != null)
                WritePointer(toAdress.area, toAdress.offset, fromOffset);
            else if (fromOffset >= 0)
                Write(fromOffset, (w) => w.Write(FireBin.Pointer.NullPointer));
            else
                writer.Write(FireBin.Pointer.NullPointer);
        }

        public void WritePointer(Area toArea, long toOffset, long fromOffset = -1) {
            bool useOffset = fromOffset >= 0;

            m_pointers.Add(new Pointer() {
                fromArea = this,
                toArea = toArea,
                fromOffset = useOffset ? fromOffset : Offset,
                toOffset = toOffset,
            });

            if (useOffset)
                Write(fromOffset, (w) => w.Write(FireBin.Pointer.NullPointer));
            else
                writer.Write(FireBin.Pointer.NullPointer);
        }
    }
}
