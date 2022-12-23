using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Reflection;

namespace FireYaml {
    public static class AssetInstance {
        private static ulong m_nextId = 1;

        public static ulong PopId() => m_nextId++;
    }

    public interface IFile {
        public const string NotPrefab = "";

        [Engine.Close] ulong assetInstance { get; set; }

        [Engine.Close] int fileId { get; set; }

        [Engine.Close] string prefabId { get; set; }

        bool IsPrefab => prefabId != NotPrefab;

        public static int GetFileId(ref object target) {
            var type = target.GetType();
            var ifileType = typeof(IFile);

            if (type.GetInterface(nameof(IFile)) == null)
                return -1;

            var prop = ifileType.GetProperty(nameof(IFile.fileId));
            if (prop == null)
                throw new Exception("IFile.fileId not found");

            var fileId = prop.GetValue(target);
            if (fileId == null)
                throw new Exception("IFile.fileId is null");

            return (int)fileId;
        }

        public static void SetFileId(int fileId, ref object target) {
            var type = target.GetType();
            var ifileType = typeof(IFile);

            if (type.GetInterface(nameof(IFile)) == null)
                return;

            var prop = ifileType.GetProperty(nameof(IFile.fileId));
            if (prop == null)
                throw new Exception("IFile.fileId not found");

            prop.SetValue(target, fileId);
        }

        public static string GetPrefabId(ref object target) {
            var type = target.GetType();
            var ifileType = typeof(IFile);

            if (type.GetInterface(nameof(IFile)) == null)
                return IFile.NotPrefab;

            var prop = ifileType.GetProperty(nameof(IFile.prefabId));
            if (prop == null)
                throw new Exception("IFile.fileId not found");

            var fileId = prop.GetValue(target);
            if (fileId == null)
                throw new Exception("IFile.fileId is null");

            return (string)fileId;
        }

        public static void SetPrefabId(string prefabId, ref object target) {
            var type = target.GetType();
            var ifileType = typeof(IFile);

            if (type.GetInterface(nameof(IFile)) == null)
                return;

            var prop = ifileType.GetProperty(nameof(IFile.prefabId));
            if (prop == null)
                throw new Exception("IFile.prefabId not found");

            prop.SetValue(target, prefabId);
        }

        public static ulong GetAssetInstance(ref object target) {
            var type = target.GetType();
            var ifileType = typeof(IFile);

            if (type.GetInterface(nameof(IFile)) == null)
                return 0;

            var prop = ifileType.GetProperty(nameof(IFile.assetInstance));
            if (prop == null)
                throw new Exception("IFile.fileId not found");

            var assetId = prop.GetValue(target);
            return (ulong)assetId;
        }

        public static void SetAssetInstance(ulong assetInst, ref object target) {
            var type = target.GetType();
            var ifileType = typeof(IFile);

            if (type.GetInterface(nameof(IFile)) == null)
                return;

            var prop = ifileType.GetProperty(nameof(IFile.assetInstance));
            if (prop == null)
                throw new Exception("IFile.assetInstance not found");

            prop.SetValue(target, assetInst);
        }

    }
}
