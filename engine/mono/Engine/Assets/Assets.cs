using System;
using System.IO;
using System.Reflection;
using System.Collections.Generic;
using System.Text;
using EngineMono;
using System.Runtime.InteropServices;

using EngineDll;
using FireYaml;

namespace Engine {

    public static class StringExt {

        /// <summary>
        /// Если assetID, это GUID, парсит GUID и возвращает его hash, иначе возвращает hash строки.
        /// <br/>
        /// Таким образом AssetID может быть как GUID, так и любой уникальной строкой.
        /// </summary>
        /// <param name="assetID"></param>
        /// <returns></returns>
        public static int GetAssetIDHash(this string assetID) {
            Guid assetIDGuid;
            if (Guid.TryParse(assetID, out assetIDGuid))
                return assetIDGuid.GetHashCode();

            return assetID.GetHashCode();
        }
    }

    public sealed class Assets {

        public static string M_Default = "cd882168-97ef-4799-98a2-c48a0955ba32";
        public static string editor_settings = "d41d19d7-fef9-4034-a3e3-6589d657d863";


        public static event Action<int, IAsset> AssetUpdateEvent;
        public static event Action<int> TextureAssetUpdateEvent;

        public static Dictionary<int, string> m_assetSources = new Dictionary<int, string>() {
            { typeof(Image).FullName.GetHashCode(), "|.png|.jpg|" },
            { typeof(Mesh).FullName.GetHashCode(), "|.obj|" },
            { typeof(AudioAsset).FullName.GetHashCode(), "|.wav|" },
        };

        private static Dictionary<int, IAsset> m_loadedAssets = new Dictionary<int, IAsset>();

        public static void SetLoadedAsset(IAsset asset) {
            if (asset.assetId == "")
                throw new ArgumentException($"Assets.SetLoadedAsset: The asset: '{asset.GetType().Name}' has empty assetId.");

            m_loadedAssets[asset.assetIdHash] = asset;
        }

        public static TAsset GetLoaded<TAsset>(int assetIdHash) where TAsset: class, IAsset {
            var iasset = GetLoadedAsset(assetIdHash);
            if (iasset == null)
                return null;

            var asset = iasset as TAsset;
            if (asset == null)
                throw new ArgumentException($"Assets.GetLoaded: The asset type: '{iasset.GetType().Name}' does not match the specified type: '{typeof(TAsset).Name}'.");

            return asset;
        }

        public static IAsset GetLoadedAsset(int assetIdHash) {
            if(m_loadedAssets.ContainsKey(assetIdHash))
                return m_loadedAssets[assetIdHash];

            return null;
        }

        public static void Save(int assetIdHash) {
            Dll.Assets.Save(Game.gameRef, assetIdHash);
        }

        public static void NotifyAssetUpdate(int assetIdHash) {
            var assetType = AssetStore.GetAssetType(assetIdHash);
            if (assetType == null)
                return;

            if (assetType == typeof(Texture)) {
                TextureAssetUpdateEvent?.Invoke(assetIdHash);
                return;
            }
        }

        public static CppRef PushCppAsset<TAsset>(string assetId, int assetIdHash) {
            var type = typeof(TAsset);
            return PushCppAsset(type, assetId, assetIdHash);
        }

        public static CppRef PushCppAsset(Type type, string assetId, int assetIdHash) {
            if (type == typeof(Image))
                return Dll.Image.PushAsset(Game.gameRef, assetId, assetIdHash);

            if (type == typeof(Texture))
                return Dll.Texture.PushAsset(Game.gameRef, assetId, assetIdHash);

            if (type == typeof(StaticMaterial))
                return Dll.Material.PushAsset(Game.gameRef, assetId, assetIdHash);

            if (type == typeof(Mesh))
                return Dll.MeshAsset.PushAsset(Game.gameRef, assetId, assetIdHash);

            if (type == typeof(Scene))
                return Dll.Scene.PushAsset(Game.gameRef, assetId, assetIdHash);

            if (type == typeof(AudioAsset))
                return Dll.AudioAsset.PushAsset(Game.gameRef, assetId, assetIdHash);

            var csOnly = type.GetCustomAttribute<CsOnly>();
            if (csOnly != null)
                return Dll.PureAsset.PushAsset(Game.gameRef, assetId, assetIdHash);

            throw new Exception($"PushCppAsset: Cannot push cpp asset of specified type: {type.Name}.");
        }

        public static bool cpp_Load(CppRef cppRef, int assetIdHash) {
            if (AssetStore.IsRuntimeAsset(assetIdHash))
                return false;

            try {
                var asset = CreateAssetWrapper(assetIdHash, cppRef);
                asset.LoadAsset();
                return true;

            } catch (Exception e) {
                Console.WriteLine("Assets.Load:");

                if (e.InnerException != null) {
                    Console.WriteLine(e.InnerException.Message);
                    Console.WriteLine(e.InnerException.StackTrace);
                }
                Console.WriteLine(e.Message);
                Console.WriteLine(e.StackTrace);
                return false;
            }
        }

        public static void cpp_Reload(CppRef cppRef, int assetIdHash) {
            if (AssetStore.IsRuntimeAsset(assetIdHash))
                return;

            var asset = GetLoadedAsset(assetIdHash);
            if (asset == null)
                // throw new Exception($"Assets.Save: The asset is not loaded.");
                asset = CreateAssetWrapper(assetIdHash, cppRef);

            asset.ReloadAsset();

            AssetUpdateEvent?.Invoke(assetIdHash, asset);
        }

        public static void cpp_Save(CppRef cppRef, int assetIdHash) {
            try {
                if (AssetStore.IsRuntimeAsset(assetIdHash))
                    return;

                var asset = GetLoadedAsset(assetIdHash);
                if (asset == null)
                    // throw new Exception($"Assets.Save: The asset is not loaded.");
                    asset = CreateAssetWrapper(assetIdHash, cppRef);

                asset.SaveAsset();
                
            } catch (Exception e) {
                Console.WriteLine("Assets.Load:");

                if (e.InnerException != null) {
                    Console.WriteLine(e.InnerException.Message);
                    Console.WriteLine(e.InnerException.StackTrace);
                }
                Console.WriteLine(e.Message);
                Console.WriteLine(e.StackTrace);
            }
        }

        public static string FindSourceFile(Type assetType, string assetPath) {
            var typeName = assetType.FullName;
            var typeNameHash = typeName.GetHashCode();

            if (!m_assetSources.ContainsKey(typeNameHash))
                throw new Exception($"Assets.FindSourceFile: unknown source for '{typeName}'");

            var filter = m_assetSources[typeNameHash];
            var dirPath = Path.GetDirectoryName(assetPath);
            var assetName = Path.GetFileNameWithoutExtension(assetPath);

            foreach (string filePath in Directory.EnumerateFiles(dirPath)) {
                var fileName = Path.GetFileNameWithoutExtension(filePath);
                var ext = Path.GetExtension(filePath);
                
                if (assetName == fileName && filter.Contains($"|{ext}|"))
                    return filePath;
            }
            return "";
        }

        //public static int GetStringHash(ulong ptr, ulong length) {
        //    var assetId = ReadCString(ptr, length);
        //    return assetId.GetHashCode();
        //}

        /// <summary>
        /// Создает IAsset только для дабавленного в AssetStore ассета.
        /// </summary>
        /// <param name="assetIdHash">Hash ID asset-а</param>
        /// <param name="cppRef">Ссылка на Cpp-объект, нужна только в том случае, 
        /// если asset загружается как файл для asset-а созданного в C++</param>
        public static IAsset CreateAssetWrapper(int assetIdHash, CppRef cppRef) {
        
            var assetType = AssetStore.GetAssetType(assetIdHash);
            if (assetType == typeof(Actor))
                assetType = typeof(Prefab);

            var iassetName = nameof(IAsset);
            var iassetInterface = assetType.GetInterface(iassetName);
            if (iassetInterface == null)
                throw new Exception($"Assets.Load(): Asset Type {assetType.FullName} not implement {iassetName} interface");

            var assetId = AssetStore.GetAssetGuid(assetIdHash);
            var assetObj = FireReader.CreateInstance(assetType);
            var asset = assetObj as IAsset;

            asset.Init(assetId, cppRef);

            return asset;
        }
        
        public static void MakeDirty(int assetIdHash) {
            Dll.Assets.MakeDirty(Game.gameRef, assetIdHash);
        }

        public static bool IsDirty(int assetIdHash) {
            return Dll.Assets.IsDirty(Game.gameRef, assetIdHash);
        }

        public static Type GetAssetTypeByExt(string ext) {
            switch (ext) {
                case ".prefab":
                    return typeof(Engine.Actor);

                case ".scene":
                    return typeof(Engine.Scene);

                case ".tex":
                    return typeof(Engine.Texture);

                case ".obj":
                    return typeof(Engine.Mesh);

                case ".png":
                case ".jpg":
                    return typeof(Engine.Image);

                case ".mat":
                    return typeof(Engine.StaticMaterial);

                case ".wav":
                    return typeof(Engine.AudioAsset);

                default:
                    Console.WriteLine($"CreatAsset: Unknown asset extention: {ext}");
                    return null;
            }
        }

        
        public static string GetAssetExtBySourceExt(string ext) {
            switch (ext) {
                case ".obj":
                    return ".mesh";

                case ".png":
                case ".jpg":
                    return ".image";

                case ".wav":
                    return ".audio";

                default:
                    return "";
            }
        }

        public static string ReadCString(ulong ptr, ulong length) {
            string str = "";
            unsafe {
                byte[] bytes = new byte[length];
                byte* bptr = (byte*)ptr;
                for (ulong i = 0; i < length; i++, bptr++) {
                    bytes[i] = *bptr;
                }
                str = Encoding.UTF8.GetString(bytes);
            }
            return str;
        }

        public static string ReadCString(ulong ptr) {
            string str = "";
            unsafe {
                var bytes = new List<byte>();
                byte* byte_ptr = (byte*)ptr;
                for (ulong i = 0; i < 1024; i++, byte_ptr++) {
                    byte m_byte = *byte_ptr;
                    if(m_byte == 0)
                        break;
                    bytes.Add(m_byte);
                }
                str = Encoding.UTF8.GetString(bytes.ToArray());
            }
            return str;
        }

    }


}
