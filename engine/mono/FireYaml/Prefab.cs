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
        /// <-

        public Prefab() { }

        public Prefab(string assetId) { this.assetId = assetId; }
        
        public Actor Instanciate() {
            return new FireYaml.Deserializer(assetId).Instanciate<Actor>();
        }


        public void LoadAsset() {
            Console.WriteLine($"NotImplemented: {nameof(Prefab.LoadAsset)}");
        }

        public void ReloadAsset() {

        }

        private void OnAfterReload(int assetId, FireYaml.IAsset asset) {

        }
    }
}