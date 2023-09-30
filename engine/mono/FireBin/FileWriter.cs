using System;
using System.IO;
using System.Collections.Generic;
using System.Collections;
using System.Linq;
using System.Text;


namespace FireBin {
    public class FileWriter {
        private FireBin.Data m_data = new Data();

        public FileWriter(FireBin.Data data) {
            m_data = data;
        }

        public void Write(FileStream fileStream) {
            var writer = new BinaryWriter(fileStream);

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

                area.InfilePosition = (int)writer.BaseStream.Position;
                writer.Write(area.stream.ToArray());

                writer.SaveOffset(areaInfoOffset, w => {
                    w.Write(area.InfilePosition);
                    w.Write(areaLength);
                });
            }

            m_WriteReferences(writer);

            for (int i = 0; i < m_data.AreasCount; i++)
                m_data[i].InfilePosition = 0;
        }

        private void m_WriteReferences(BinaryWriter writer) {
            int writedRefsCount = 0;

            m_data.DistinctPointers();

            for (int i = 0; i < m_data.PointersCount; ++i) {
                var fromPtr = m_data.GetPointer(i);
                if (fromPtr.offset == Pointer.NullOffset)
                    continue;

                Area.WritePointerData(writer, fromPtr);

                writedRefsCount++;
            }

            Console.WriteLine($"Writed Refs Count: {writedRefsCount}");
        }
    }
}
