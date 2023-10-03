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

    public sealed class Assets {

        public static string M_Default = "cd882168-97ef-4799-98a2-c48a0955ba32";
        public static string editor_settings = "d41d19d7-fef9-4034-a3e3-6589d657d863";


        public static event Action<int, IAsset> AfterReloadEvent;

        public static Dictionary<int, string> m_assetSources = new Dictionary<int, string>() {
            { typeof(Image).FullName.GetHashCode(), "|.png|.jpg|" },
            { typeof(StaticMesh).FullName.GetHashCode(), "|.obj|" },
        };

        public static bool Load(int assetIdHash, CppRef cppRef) {
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

        public static void Reload(int assetIdHash) {
            var asset = CreateAssetWrapper(assetIdHash, CppRef.NullRef);
            asset.ReloadAsset();

            AfterReloadEvent?.Invoke(assetIdHash, asset);
        }

        public static void Save(int assetIdHash) {
            var asset = CreateAssetWrapper(assetIdHash, CppRef.NullRef);
            asset.SaveAsset();
        }

        public static void InitNewAsset(Type assetType, string assetPath, string assetGuid, string assetSourcePath = "") {

            if(!File.Exists(assetPath))
                throw new FileNotFoundException($"Assets.Create: {assetPath}");

            if (assetType == typeof(Engine.Actor)) {
                m_InitNewPrefab(assetPath, assetGuid);
                return;
            }

            var asset = FireReader.CreateInstance(assetType);

            if (FireWriter.IsAsset(assetType)) 
                FireReader.InitIAsset(ref asset, assetGuid, 0);
            
            if (FireWriter.IsAssetWithSource(assetType)) {
                if (assetSourcePath == "")
                    throw new Exception("Assets.Create: Asset with source need a sourcePath");

                var ext = Path.GetExtension(assetSourcePath);

                FireReader.InitISourceAsset(ref asset, ext);
            }

            AssetStore.Instance.WriteAsset(assetPath, asset);
        }

        private static void m_InitNewPrefab(string assetPath, string assetGuid) {
            
            var filesCount = 1;
            var scriptName = nameof(Engine.Actor);
            var scriptGuid = GUIDAttribute.GetGuid(typeof(Engine.Actor));

            var text = File.ReadAllText(assetPath);
            var values = new YamlValues().LoadFromText(text);

            values.SetValue(".file0.assetId", new YamlValue(YamlValue.Type.AssetId, assetGuid));
            values.SetValue(".file0.files", new YamlValue(YamlValue.Type.Var, $"{filesCount}"));

            values.AddValue(".file1!script", new YamlValue(YamlValue.Type.Var, $"{scriptName}"));
            values.AddValue(".file1!scriptId", new YamlValue(YamlValue.Type.AssetId, $"{scriptGuid}"));

            File.WriteAllText(assetPath, values.ToSortedText());
        }

        public static string FindSourceFile(Type assetType, string assetPath) {
            var typeName = assetType.FullName;
            var typeNameHash = typeName.GetHashCode();

            if (!m_assetSources.ContainsKey(typeNameHash))
                throw new Exception($"Assets.Create: unknown source extentions for '{typeName}'");

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

        public static int GetStringHash(ulong ptr, ulong length) {
            var assetId = ReadCString(ptr, length);
            return assetId.GetHashCode();
        }

        /// <summary>
        /// Создает IAsset только для дабавленного в AssetStore ассета.
        /// </summary>
        /// <param name="assetIdHash">Hash ID asset-а</param>
        /// <param name="cppRef">Ссылка на Cpp-объект, нужна только в том случае, 
        /// если asset загружается как файл для asset-а созданного в C++</param>
        public static IAsset CreateAssetWrapper(int assetIdHash, CppRef cppRef) {
            var assetStore = AssetStore.Instance;
        
            var assetType = assetStore.GetAssetType(assetIdHash);

            var iassetName = nameof(IAsset);
            var iassetInterface = assetType.GetInterface(iassetName);
            if (iassetInterface == null)
                throw new Exception($"Assets.Load(): Asset Type {assetType.FullName} not implement {iassetName} interface");

            var asset = FireReader.CreateInstance(assetType);

            var assetIdStr = assetStore.GetAssetGuid(assetIdHash);

            FireReader.InitIAsset(ref asset, assetIdStr, cppRef);

            return asset as IAsset;
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
                    return typeof(Engine.StaticMesh);

                case ".png":
                case ".jpg":
                    return typeof(Engine.Image);

                case ".material":
                    return typeof(Engine.StaticMaterial);

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
