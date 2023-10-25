using System;
using System.IO;
using System.Collections.Generic;
using System.Text;

using EngineDll;
using FireYaml;

namespace Engine {

    public class ImageData : AssetDataBase {
        public string ext = "";
        public int width;
        public int height;
    }

    [GUID("37da25c5-f1ed-4976-b924-0bb31816735d", typeof(Image))]
    public class Image : AssetBase<Image, ImageData>, IAsset, ISourceAsset {
        public string ext { get => m_data.ext; set => m_data.ext = value; }
        [Close] public int width { get => m_data.width; set => m_data.width = value; }
        [Close] public int height { get => m_data.height; set => m_data.height = value; }

        public override void ReloadAsset() {
            LogReload();

            cppRef = Dll.Assets.Get(Game.gameRef, assetIdHash);
            if (cppRef.value == 0)
                throw new Exception("Asset not loaded");

            AssetStore.GetAssetDeserializer(assetIdHash).InstanciateToWithoutLoad(this);

            var selfPath = AssetStore.GetAssetPath(assetIdHash);
            var sourcePath = Path.ChangeExtension(selfPath, ext);

            int w = 0, h = 0;
            Dll.Image.Init(Game.gameRef, cppRef, sourcePath, ref w, ref h);
            width = w;
            height = h;
        }

    }
}
