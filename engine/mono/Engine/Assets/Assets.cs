using System;
using System.Reflection;
using System.Collections.Generic;
using System.Text;
using EngineMono;
using System.Runtime.InteropServices;

using EngineDll;

namespace Engine {

    public sealed class Assets {

        public static event Action<int, FireYaml.IAsset> AfterReloadEvent;

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
        public static FireYaml.IAsset CreateAssetWrapper(int assetIdHash, CppRef cppRef) {
            var assetStore = FireYaml.AssetStore.Instance;

            var typeId = assetStore.GetAssetScriptId(assetIdHash);
            var typeName = assetStore.GetTypeFullName(typeId);
            var assetType = Type.GetType(typeName);

            var iassetName = nameof(FireYaml.IAsset);
            var iassetInterface = assetType.GetInterface(iassetName);
            if (iassetInterface == null)
                throw new Exception($"Assets.Load(): Asset {typeName} not implement {iassetName} interface");

            var asset = FireYaml.FireReader.CreateInstance(assetType);

            var assetIdStr = assetStore.GetAssetId(assetIdHash);

            FireYaml.FireReader.InitIAsset(ref asset, assetIdStr, cppRef);

            return asset as FireYaml.IAsset;
        }
        
        public static void MakeDirty(int assetIdHash) {
            Dll.Assets.MakeDirty(Game.gameRef, assetIdHash);
        }

        public static bool IsDirty(int assetIdHash) {
            return Dll.Assets.IsDirty(Game.gameRef, assetIdHash);
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
