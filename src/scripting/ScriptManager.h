/*
    This file is part of Element
    Copyright (C) 2020  Kushview, LLC.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#pragma once

#include "scripting/ScriptDescription.h"

namespace Element {

using ScriptArray = Array<ScriptDescription>;

class ScriptManager final
{
public:
    ScriptManager();
    ~ScriptManager();

    void scanDefaultLocation();
    
    int getNumScripts() const;
    ScriptDescription getScript (int) const;

    const ScriptArray& getScriptsDSP() const;

    static File getSystemScriptsDir();
    static File getHomeScriptsDir();
    static File getUserScriptsDir();

private:
    class Registry;
    std::unique_ptr<Registry> registry;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScriptManager);
};

}
