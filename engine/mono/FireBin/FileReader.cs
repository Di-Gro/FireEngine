using System;
using System.IO;
using System.Collections.Generic;
using System.Text;

//using PointerType = System.Int32;

namespace FireBin {

    public class FileReader {

        private FireBin.Data m_data;
        private BinaryReader m_reader;

        public FileReader() { }

        public FireBin.Data Read(FileStream fileStream) {
            m_data = new Data();
            m_reader = new BinaryReader(fileStream);

            var fbinHeader = new string(m_reader.ReadChars(8));

            for (int i = 0; i < m_data.AreasCount; i++) {
                var area = m_data[i];

                area.InfilePosition = m_reader.ReadInt32();
                var length = m_reader.ReadInt32();

                area.writer.Write(m_reader.SaveOffset(area.InfilePosition, r => r.ReadBytes(length)));
            }

            var lastArea = m_data[m_data.AreasCount - 1];
            var ptrsPos = lastArea.InfilePosition + lastArea.Length;

            ReadPointers(ptrsPos);

            m_data.CollectNames();
            m_data.CollectAssetRefs();

            for (int i = 0; i < m_data.AreasCount; i++)
                m_data[i].InfilePosition = 0;

            var res = m_data;

            m_data = null;
            m_reader = null;

            return res;
        }

        public void ReadPointers(int position) {
            var length = m_reader.BaseStream.Length;
            var count = (length - position) / Pointer.Size;

            var prevPos = m_reader.BaseStream.Position;
            m_reader.BaseStream.Position = position; 

            for (int i = 0; i < count; i++) {
                var fromPtr = Area.ReadPointerData(m_reader);

                m_data.AddPointer(fromPtr);
            }
            m_reader.BaseStream.Position = prevPos;
        }

    }
}
