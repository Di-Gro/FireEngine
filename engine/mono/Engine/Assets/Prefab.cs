using YamlDotNet.Serialization;
using YamlDotNet.Serialization.NamingConventions;
using YamlDotNet.RepresentationModel;
using System.Numerics;
using System.Collections;
using System.Collections.Generic;
using System.Reflection;
using System.Globalization;
using System.Diagnostics;
using System;

namespace Engine {

    [GUID("eec5f7b8-5d7b-4d78-b1a8-d69f21459e73", typeof(Prefab))]
    sealed class Prefab : FireYaml.IAsset {

        /// FireYaml.IAsset
        [Open] public string assetId { get; private set; } = "0000000000";
        public int assetIdHash { get; private set; } = 0;
        [Close] public CppRef cppRef { get; private set; } = CppRef.NullRef;
        /// <-

        public Prefab() { }

        public Prefab(string assetId) { 
            this.assetId = assetId; 
            assetIdHash = assetId.GetHashCode(); 
        }

        public Actor Instanciate() {
            return new FireYaml.FireReader(assetId).Instanciate<Actor>();
        }

        public void LoadAsset() {
            assetIdHash = assetId.GetHashCode();
        }

        public void ReloadAsset() { }

        public void SaveAsset() { }

    }
}