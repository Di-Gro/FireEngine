using System;
using System.IO;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

using EngineDll;
using FireYaml;

namespace Engine {

    public static class Clipboard {

        private static YamlValues m_values = null;
        private static Type m_sourceType;

        public static void Push(CsRef csRef) {
            var source = CppLinked.GetObjectByRef(csRef);
            if(source == null)
                return;

            var serializer = new FireYaml.FireWriter(ignoreExistingIds: true, writeNewIds: false);
            serializer.Serialize(source);

            if (!serializer.Result)
                throw new Exception($"Can not update asset: Serialization failed.");

            m_values = serializer.Values;

            var text = m_values.ToSortedText();
            var path = $"{AssetStore.Instance.EditorPath}/tmp/Clipboard.yml";

            File.WriteAllText(path, text);

            var scriptId = m_values.GetValue(".file1!scriptId", "");
            var typeFullName = AssetStore.Instance.GetTypeFullName(scriptId);

            m_sourceType = Type.GetType(typeFullName);
        }

        public static CppRef Peek() {
            if (m_values == null)
                return CppRef.NullRef;

            try {
                var target = new FireReader(m_values).Instanciate();

                var ICppLinkedType = target.GetType().GetInterface(nameof(ICppLinked));
                if (ICppLinkedType == null)
                    throw new Exception("Clipboard target not implement ICppLinked interface.");

                var cppLinked = target as ICppLinked;
                return cppLinked.cppRef;

            } catch (Exception e) {
                WriteException(e);
            }
            return CppRef.NullRef;
        }

        public static bool IsAssignable(int typeIdHash) {
            return m_ForClipboardSource(typeIdHash, (sourceType, targetType) => targetType.IsAssignableFrom(sourceType));
        }

        public static bool IsSameType(int typeIdHash) {
            return m_ForClipboardSource(typeIdHash, (sourceType, targetType) => sourceType == targetType);
        }

        private static bool m_ForClipboardSource(int typeIdHash, Func<Type, Type, bool> func) {
            if (m_values == null)
                return false;

            try {
                var fullName = AssetStore.Instance.GetTypeFullName(typeIdHash);
                var targetType = Type.GetType(fullName);
                if (targetType == null)
                    throw new Exception($"Clipboard don't known '{fullName}' type.");

                return func(m_sourceType, targetType);

            } catch (Exception e) {
                WriteException(e);
                return false;
            }
        }

        private static void WriteException(Exception e) {
            Console.WriteLine("Game Exception:");

            if (e.InnerException != null) {
                Console.WriteLine(e.InnerException.Message);
                Console.WriteLine(e.InnerException.StackTrace);
            }
            Console.WriteLine(e.Message);
            Console.WriteLine(e.StackTrace);
        }

    }

}
