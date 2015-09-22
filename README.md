#Cinder-Luabind
Lua 5.2 and Luabind 0.9.1

Lua is a light-weight scripting language that can be used to add dynamic runtime logic capabilities to your programs. Luabind is a library that makes binding your c++ code to lua fast and easy.

To use Cinder-Luabind, include the block in your project and you'll need to adjust two settings

1. Build Settings >> Always Search User Paths >> Yes
2. Place the following includes in the _Prefix.pch file BEFORE cinder
```c++
extern "C"
{
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}
#include <luabind/luabind.hpp>
```

#Notes

Luabind is no longer officially supported and is now supported by a small community.
This build comes from [rpavlik](https://github.com/rpavlik/luabind)  


