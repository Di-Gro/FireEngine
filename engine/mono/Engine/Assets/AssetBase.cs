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
        private string m_assetId = "0000000000";
        public string assetId { get => m_assetId; protected set { m_assetId = value; assetIdHash = m_assetId.GetAssetIDHash(); } } 
        public int assetIdHash { get; protected set; }
        public CppRef cppRef { get => m_data.cppRef; protected set => m_data.cppRef = value; }

        protected TAssetData m_data = new TAssetData();
        protected TAsset m_assetInstance = null;

        public virtual void LoadAsset() {
            if (m_TakeLoadedInstance())
                return;

            m_PushThisInstance();
            m_PushCppAsset();
            ReloadAsset();
        }

        public virtual void ReloadAsset() { }

        public virtual void SaveAsset() {
            AssetStore.WriteAsset(assetIdHash, this);
        }

        protected bool m_TakeLoadedInstance() {
            m_assetInstance = Assets.GetLoaded<TAsset>(assetIdHash);
            if (m_assetInstance != null) {
                var assetBase = m_assetInstance as AssetBase<TAsset, TAssetData>;
                if (assetBase == null)
                    throw new Exception($"loadedAsset as AssetBase<{typeof(TAsset).Name}, {typeof(TAssetData).Name}> == null");

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
            if (cppRef.value == 0)
                cppRef = Assets.PushCppAsset<TAsset>(assetId, assetIdHash);

            // if (cppRef.value != 0)
            //     throw new Exception($"AssetBase<{typeof(TAsset).Name}, {typeof(TAssetData).Name}>: The cpp asset already exists.");
        }

        public int GetAssetsListHash<T>(List<T> list) where T : IAsset {
            long value = 0;
            foreach (var asset in list)
                value += asset != null ? asset.assetIdHash : -1111;

            return value.GetHashCode();
        }

    }
}