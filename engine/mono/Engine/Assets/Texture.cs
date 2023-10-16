﻿using System;
using System.Collections.Generic;
using System.Text;

using EngineDll;
using FireYaml;

namespace Engine {

    public class TextureData : AssetDataBase {
        public Image image;
        public uint width = 128;
        public uint height = 128;
        public int imageIdHash = 0;
    }

    [GUID("81b31ac1-86d8-4d99-aff6-324c5f987b15", typeof(Texture))]
    public class Texture : AssetBase<Texture, TextureData>, IAsset, IEditorUIDrawer {

        public Image image { get => m_data.image; set => m_data.image = value; }
        public uint width { get => m_data.width; set => m_data.width = value; }
        public uint height { get => m_data.height; set => m_data.height = value; }

        public override void ReloadAsset() {
            cppRef = Dll.Assets.Get(Game.gameRef, assetIdHash);
            if(cppRef.value == 0)
                throw new Exception("Asset not loaded");

            AssetStore.GetAssetDeserializer(assetIdHash).InstanciateToWithoutLoad(this);

            m_data.imageIdHash = image == null ? 0 : image.assetIdHash;
            m_InitTexture();
        }

        public void OnDrawUI() {
            int hash = image == null ? 0 : image.assetIdHash;
            if (m_data.imageIdHash != hash) {
                m_data.imageIdHash = hash;
                m_InitTexture();
                Assets.NotifyAssetUpdate(assetIdHash);
            }
        }

        private void m_InitTexture() {
            if (image == null)
                Dll.Texture.Init(Game.gameRef, cppRef, width, height);
            else
                Dll.Texture.InitFromImage(Game.gameRef, cppRef, image.cppRef);
        }

    }
}
