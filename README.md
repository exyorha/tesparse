# tesparse

tesparse implements a parser for Morrowind game data files (ESM and ESP files)
in a flexible way, using a XML description file. 

While implementation of the later games is intended eventually, it will require
group-based parsing to be implemented first.

tesparse, by design, does *not* implement record merging, as it is highly
irregular and game-specific.

See tesparse-cli or an usage example.

Please note that currently the only supported platform is Windows.

# Building

tesparse may be built using normal CMake procedures, and is generally
intended to included into an outer project as a submodule or by any other
means.


# Licensing

tesparse is licensed under the terms of the MIT license (see LICENSE).
