﻿using System;
using System.Collections.Generic;
using System.Text;

namespace FireYaml {

    public interface IAsset {

        string assetId { get; }

        void LoadAsset();

    }
}