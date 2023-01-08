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

        public static void Load(int assetId) {
            var asset = m_CreateAssetWrapper(assetId);
            asset.LoadAsset();
        }

        public static void Reload(int assetId) {
            var asset = m_CreateAssetWrapper(assetId);
            asset.ReloadAsset();

            AfterReloadEvent?.Invoke(assetId, asset);
        }

        public static int GetStringHash(ulong ptr, ulong length) {
            var assetId = ReadCString(ptr, length);
            return assetId.GetHashCode();
        }

        private static FireYaml.IAsset m_CreateAssetWrapper(int assetId) {
            var assetStore = FireYaml.AssetStore.Instance;

            var typeId = assetStore.GetAssetScriptId(assetId);
            var typeName = assetStore.GetTypeFullName(typeId);
            var assetType = Type.GetType(typeName);

            var iassetName = nameof(FireYaml.IAsset);
            var iassetInterface = assetType.GetInterface(iassetName);
            if (iassetInterface == null)
                throw new Exception($"Assets.Load(): Asset {typeName} not implement {iassetName} interface");

            var asset = FireYaml.Deserializer.CreateInstance(assetType);

            var assetIdStr = assetStore.GetAssetId(assetId);
            m_SetAssetId(asset, assetIdStr);

            return asset as FireYaml.IAsset;
        }

        private static void m_SetAssetId(object asset, string assetId) {
            var fieldName = nameof(FireYaml.IAsset.assetId);
            var serializer = new Engine.SerializerBase();
            var field = FireYaml.Serializer.GetField(fieldName, asset.GetType(), asset, serializer);

            field.SetValue(assetId);
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
