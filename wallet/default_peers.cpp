// Copyright 2019 The Grimm Team
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and

#include "default_peers.h"

namespace grimm
{
    std::vector<std::string> getDefaultPeers()
    {
        std::vector<std::string> result
        {
#ifdef GRIMM_TESTNET

#else
            "usmainnet.grimmw.com:8385",
            "amsmainnet.grimmw.com:8385",
            "bangmainnet.grimmw.com:8385",
            "eumainnet.grimmw.com:8385"
#endif
        };

        return result;
    }
}
