using System;
using System.IO;
using System.Collections.Generic;
using System.Text;

namespace FireBin {
    public class AssetReader {
        private BinaryReader m_reader;

        public AssetReader(FileStream stream) {
            m_reader = new BinaryReader(stream);
        }

        public AssetHeader ReadHeader() {
            m_reader.BaseStream.Position = 0;

            m_reader.ReadChars(8);

            var assetHeader = new AssetHeader();

            assetHeader.assetID = m_reader.ReadString();
            assetHeader.sourceExt = m_reader.ReadString();
            assetHeader.scriptID = m_reader.ReadString();

            return assetHeader;
        }

        public Data ReadData() {
            ReadHeader();

            return new DataReader().Read(m_reader);
        }
    }
}
