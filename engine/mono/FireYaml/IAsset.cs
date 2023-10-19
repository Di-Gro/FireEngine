using System;
using System.Collections.Generic;
using System.Text;

using Engine;

namespace FireYaml {

    public interface IAsset {

        string assetId { get; }
        int assetIdHash { get; }
        CppRef cppRef { get; }

        void LoadAsset();
        void ReloadAsset();
        void SaveAsset();
    }

    public interface IAssetEditorListener {
        void OnEditAsset();
    }

    public interface ISourceAsset {

        string ext { get; }

    }
}
