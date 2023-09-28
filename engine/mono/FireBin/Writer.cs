using System;
using System.IO;
using System.Collections.Generic;
using System.Collections;
using System.Linq;
using System.Text;


namespace FireBin {
    public class Writer {
        private FireBin.Data m_data = new Data();

        public Writer(FireBin.Data data) {
            m_data = data;
        }

        public void Write(FileStream fileStream) {
            var writer = new BinaryWriter(fileStream);

            writer.Write("FBINv001".ToArray());

            var areaInfoOffsets = writer.BaseStream.Position;
            for (int i = 0; i < m_data.areas.Length; i++) {
                writer.Write(Pointer.NullOffset);
                writer.Write(Pointer.NullOffset);
            }

            for (int i = 0; i < m_data.areas.Length; i++) {
                var area = m_data.areas[i];
                var areaInfoOffset = areaInfoOffsets + i * sizeof(int) * 2;
                var areaLength = (int)area.writer.BaseStream.Length;

                area.Position = (int)writer.BaseStream.Position;
                writer.Write(area.stream.ToArray());

                writer.SaveOffset(areaInfoOffset, w => {
                    w.Write(area.Position);
                    w.Write(areaLength);
                });
            }

            m_WriteReferences(writer);

            foreach (var area in m_data.areas)
                area.Position = 0;
        }

        private void m_WriteReferences(BinaryWriter writer) {
            Console.WriteLine($"Pointers.Count: {m_data.references.Count}");

            foreach (var reference in m_data.references) {
                if (reference.to.offset == Pointer.NullOffset)
                    continue;

                //Console.WriteLine($"pointer[{i++}]: {pointer.fromOffset} -> {pointer.toOffset}");

                var fromArea = m_data[reference.from.areaId];
                var toArea = m_data[reference.to.areaId];

                var fromPtr = reference.from.TranslateFrom(fromArea);
                var toPtr = reference.to.TranslateFrom(toArea);

                Area.WritePointerData(writer, fromPtr);
                writer.SaveOffset(fromPtr.offset, w => Area.WritePointerData(w, toPtr));
                
            }
        }
    }
}
