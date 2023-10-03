using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace FireBin {
    public struct AssetHeader {
        public string assetID;
        public string sourceExt;
        public string scriptID;
    }

    public class AssetWriter {
        private BinaryWriter m_writer;

        public AssetWriter(FileStream stream) {
            m_writer = new BinaryWriter(stream);
        }

        public void Write(AssetHeader asset, Data data) {
            m_writer.BaseStream.Position = 0;

            string scriptID = "";

            if (data[AreaId.Structs].Length > 0) {
                var firstStructPtr = new Pointer() { areaId = AreaId.Structs, offset = 0 };
                scriptID = new DataReader(data).ReadScriptId(firstStructPtr);
            }
            m_writer.Write("ASSETv01".ToArray());
            m_writer.Write(asset.assetID);
            m_writer.Write(asset.sourceExt);
            m_writer.Write(scriptID);

            new DataWriter(data).Write(m_writer);
        }

    }
}
