using System;
using System.IO;
using System.Collections.Generic;
using System.Text;

using PointerType = System.Int32;

namespace FireBin {
    internal static class BinaryReaderWriterExt {

        public static T SaveOffset<T>(this BinaryReader reader, long offset, Func<BinaryReader, T> readAction) {
            var prevPos = reader.BaseStream.Position;

            reader.BaseStream.Position = offset;
            var res = readAction(reader);
            reader.BaseStream.Position = prevPos;

            return res;
        } 

        public static void SaveOffset(this BinaryWriter writer, long offset, Action<BinaryWriter> writeAction) {
            var prevPos = writer.BaseStream.Position;

            writer.BaseStream.Position = offset;
            writeAction(writer);
            writer.BaseStream.Position = prevPos;
        }

    }
}
