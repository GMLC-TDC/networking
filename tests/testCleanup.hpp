/*
Copyright (c) 2017-2026,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance
for Sustainable Energy, LLC.  See the top-level NOTICE for additional details.
All rights reserved. SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "gmlc/networking/AsioContextManager.h"

#include <string>
#include <utility>
#include <vector>

class ContextCleanupGuard {
  public:
    ContextCleanupGuard() = default;

    explicit ContextCleanupGuard(std::string contextName)
    {
        add(std::move(contextName));
    }

    ~ContextCleanupGuard()
    {
        static_cast<void>(contextNames);
        try {
            gmlc::networking::AsioContextManager::closeAllContexts();
        }
        catch (...) {
            // Test cleanup should not throw during stack unwinding.
        }
    }

    void add(std::string contextName)
    {
        contextNames.push_back(std::move(contextName));
    }

  private:
    std::vector<std::string> contextNames;
};
