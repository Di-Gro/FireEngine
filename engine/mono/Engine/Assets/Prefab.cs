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
    
    [Serializable]
    sealed class Prefab : FireYaml.IAsset {

        /// FireYaml.IAsset
        [Open] public string assetId { get; private set; } = "0000000000";
        public int assetIdHash { get; private set; }
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

        public void LoadAsset() { }

        public void ReloadAsset() { }

        public void SaveAsset() { }

    }
}