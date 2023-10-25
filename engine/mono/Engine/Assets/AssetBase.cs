using System;
using System.Collections.Generic;

using EngineDll;
using FireYaml;

namespace Engine {
    public class AssetDataBase {
        public CppRef cppRef = CppRef.NullRef;
    }

    public class AssetBase<TAsset, TAssetData> : IAsset
        where TAsset : class, IAsset, new()
        where TAssetData : AssetDataBase, new() 
    {
        private static readonly bool s_printLog = false;

        private string m_assetId = "0000000000";
        public string assetId { get => m_assetId; protected set { m_assetId = value; assetIdHash = m_assetId.GetAssetIDHash(); } } 
        public int assetIdHash { get; protected set; }
        public CppRef cppRef { get => m_data.cppRef; protected set => m_data.cppRef = value; }

        public bool IsInited { get; protected set; }

        protected TAssetData m_data = new TAssetData();
        protected TAsset m_assetInstance = null;
        protected bool m_isNewInstance { get; set; }

        public virtual TAsset LoadFromAsset(string _assetId) {
            Init(_assetId, CppRef.NullRef);
            LoadAsset();
            return this as TAsset;
        }

        public virtual void Init(string _assetId, CppRef _cppRef) {
            if (IsInited)
                return;

            IsInited = true;
            assetId = _assetId;
            cppRef = _cppRef;

            if (m_TakeLoadedInstance()) 
                return;
            
            m_PushThisInstance();
            if (cppRef == CppRef.NullRef)
                m_PushCppAsset();
        }

        public string GetAssetName() {
            var asset = AssetStore.GetAsset(assetIdHash);
            if (asset == null)
                return "";

            return asset.name;
        }

        public void LogLoad() {
            if (s_printLog)
                Console.WriteLine($"L: '{assetId}' '{GetAssetName()}' '{typeof(TAsset).Name}'");
        }

        public void LogReload() {
            if (s_printLog)
                Console.WriteLine($"R: '{assetId}' '{GetAssetName()}' '{typeof(TAsset).Name}'");
        }

        public virtual void LoadAsset() {
            LogLoad();

            if (cppRef == CppRef.NullRef)
                throw new Exception($"AssetBase<{typeof(TAsset).Name}, {typeof(TAssetData).Name}>: The asset is not inited.");

            if (m_isNewInstance) {
                m_isNewInstance = false;
                ReloadAsset();
            }
        }

        public virtual void ReloadAsset() {
            LogReload();
        }

        public virtual void SaveAsset() {
            AssetStore.WriteAsset(assetIdHash, this);
        }

        protected bool m_TakeLoadedInstance() {
            m_assetInstance = Assets.GetLoaded<TAsset>(assetIdHash);
            if (m_assetInstance != null) {
                var assetBase = m_assetInstance as AssetBase<TAsset, TAssetData>;

                m_data = assetBase.m_data;

                return true;
            }
            return false;
        }

        protected void m_PushThisInstance() {
            var thisInstance = this as TAsset;
            if (thisInstance == null)
                throw new Exception($"AssetBase<{typeof(TAsset).Name}, {typeof(TAssetData).Name}> as {typeof(TAsset).Name} == null");

            m_assetInstance = thisInstance;
            Assets.SetLoadedAsset(thisInstance);
        }

        protected void m_PushCppAsset() {
            cppRef = Dll.Assets.Get(Game.gameRef, assetIdHash);
            if (cppRef.value == 0) {
                cppRef = Assets.PushCppAsset<TAsset>(assetId, assetIdHash);
                m_isNewInstance = true;
            }
        }

        public int GetAssetsListHash<T>(List<T> list) where T : IAsset {
            long value = 0;
            foreach (var asset in list)
                value += asset != null ? asset.assetIdHash : -1111;

            return value.GetHashCode();
        }

        public override string ToString() {
            return $"{typeof(TAsset).Name} {cppRef}";
        }
    }
}