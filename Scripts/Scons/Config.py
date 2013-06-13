 # Draumr build system.
 #
 # Copyright (c) 2012, Shikhin Sethi
 # All rights reserved.
 #
 # Redistribution and use in source and binary forms, with or without
 # modification, are permitted provided that the following conditions are met:
 #     * Redistributions of source code must retain the above copyright
 #       notice, this list of conditions and the following disclaimer.
 #     * Redistributions in binary form must reproduce the above copyright
 #       notice, this list of conditions and the following disclaimer in the
 #       documentation and/or other materials provided with the distribution.
 #     * Neither the name of Draumr nor the
 #       names of its contributors may be used to endorse or promote products
 #       derived from this software without specific prior written permission.
 #
 # THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 # ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 # WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 # DISCLAIMED. IN NO EVENT SHALL SHIKHIN SETHI BE LIABLE FOR ANY
 # DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 # (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 # LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 # ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 # (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 # SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# Import config parser.
import os
import ConfigParser

from SCons.Errors import StopError

class Config:
    # The expected (and parsable) parameters with options.
    Params = {
        "build": ["debug", "release"],
        "target": ["iso", "pxe", "floppy", "all"],
        "arch": ["x86_64", "i586"],
        "toolset": ["cross", "custom", "default"]
    }

    # The expected parameters with any option parsable.
    ParamsAO = {
        "pxepath",
        "CC",
        "AS",
        "LINK",
        "AR",
        "RANLIB",
        "ASFLAGS",
        "LINKFLAGS",
        "CCFLAGS",
        "PREFIX"
    }

    def __init__(self):
        # The default values.
        self.Options = {
            "arch": "x86_64",
            "build": "debug",
            "target": "all",
            "pxepath": "/tftpboot",
            "toolset": "cross",

            # Tools.
            "AS": "",
            "CC": "",
            "LINK": "",
            "AR": "",
            "RANLIB": "",  

            "PREFIX": "./Tools",

            # Flags.
            "CCFLAGS": ["-std=c99", "-Wall", "-Wextra", "-pedantic", "-Wshadow", "-Wcast-align",
                        "-Wwrite-strings", "-Wredundant-decls", "-Wnested-externs", 
                        "-Winline", "-Wno-attributes", "-Wno-deprecated-declarations", 
                        "-Wno-div-by-zero", "-Wno-endif-labels", "-Wfloat-equal", "-Wformat=2", 
                        "-Wno-format-extra-args", "-Winit-self", "-Winvalid-pch",
                        "-Wmissing-format-attribute", "-Wmissing-include-dirs",
                        "-Wno-multichar",
                        "-Wno-pointer-to-int-cast", "-Wredundant-decls",
                        "-Wshadow", "-Wno-sign-compare",
                        "-Wswitch", "-Wsystem-headers", "-Wundef",
                        "-Wno-pragmas", "-Wno-unused-but-set-parameter", "-Wno-unused-but-set-variable",
                        "-Wno-unused-result", "-Wwrite-strings", "-Wdisabled-optimization",
                        "-Werror", "-pedantic-errors", "-Wpointer-arith", "-nostdlib",
                        "-ffreestanding", "-lgcc", "-fomit-frame-pointer"],
            "ASFLAGS": ["-felf"],
            "LINKFLAGS": [],      

            # The background image.
            "BACK": "Background.bmp"
        }

        # The configuration file location.
        self.ConfigFile = "Build.cfg"

    # Could be neater, but hey, I'm not a python programmer, so I keep with basic structures.
    def Parse(self, Args):
        DefaultOptions = ["arch", "build", "target", "pxepath", "toolset"]
        ToolsOptions = ["AS", "CS", "LINK", "AR", "RANLIB", "PREFIX"]
        FlagOptions = ["CCFLAGS", "ASFLAGS", "LINKFLAGS"]

        # Get the config file name, so that we can parse it before parsing the options.
        for Key in Args.keys():
            if Key == "configfile": self.ConfigFile = Args[Key]

        ConfigFile = ConfigParser.SafeConfigParser()
        ConfigFile.read(self.ConfigFile)

        # Get build, arch, target, pxepath (and toolset - it won't actually be there, but bah, lazy code).
        for Option in DefaultOptions:
            if ConfigFile.has_option('DEFAULT', Option):
                self.Options[Option] = ConfigFile.get('DEFAULT', Option)

        # If section tools exists:
        if ConfigFile.has_section('tools'):
            # If the option toolset exists, get in the values.
            if ConfigFile.has_option('tools', 'toolset'):
                self.Options["toolset"] = ConfigFile.get('tools', 'toolset')

            # If the toolset is default or custom, first set all to default.
            if self.Options["toolset"] == "custom" or self.Options["toolset"] == "default":
                self.Options["AS"] = "nasm"
                self.Options["CC"] = "gcc"
                self.Options["LINK"] = "ld"
                self.Options["AR"] = "ar"
                self.Options["RANLIB"] = "ranlib"

            # Now, if the toolset was custom, get all the custom binaries.
            if self.Options["toolset"] == "custom":
                for Option in ToolsOptions:
                    if ConfigFile.has_option('tools', Option):
                        self.Options[Option] = ConfigFile.get('tools', Option)

            # Look for flags.
            for Option in FlagOptions:
                if ConfigFile.has_option('tools', Option):
                    self.Options[Option] += ConfigFile.get('tools', Option)

        for Key in Args.keys():
            # We have already looked at the config file option.
            if Key == "configfile": continue

            # Get the value.
            Value = Args[Key]

            # See if the value is parsable or not.
            if not self.Validate(Key, Value):
                raise StopError("Invalid value for %s parameter. Allowed values: %s." % 
                                    (Key, ", ".join(self.Params[Key])))

            # Set the new values.
            for Option in DefaultOptions:
                if Key == Option:
                    self.Options[Option] = Value

            # If it's custom, look for toolchain binaries.
            if self.Options["toolset"] == "custom":
                for Option in ToolsOptions:
                    if Key == Option:
                        self.Options[Option] = Value

            # Look for command line options.
            for Option in FlagOptions:
                if Key == Option:
                    self.Options[Option] += Value

        # If it's cross, build the locations.
        if self.Options["toolset"] == "cross":
            BinPath = os.path.sep.join([self.Options["PREFIX"], "bin"])

            self.Options["AS"] = os.path.sep.join([BinPath, "nasm"])
            self.Options["CC"] = os.path.sep.join([BinPath, "x86_64-elf-gcc"])
            self.Options["LINK"] = os.path.sep.join([BinPath, "x86_64-elf-ld"])
            self.Options["AR"] = os.path.sep.join([BinPath, "x86_64-elf-ar"])
            self.Options["RANLIB"] = os.path.sep.join([BinPath, "x86_64-elf-ranlib"])

        # If it's at debug, have no optimization, else O2 optimization.
        if self.Options["build"] == "debug":
            self.Options["CCFLAGS"] += ["-O0"]

        else:
            self.Options["CCFLAGS"] += ["-O2"]
            self.Options["ASFLAGS"] += ["-Ox"]

    # Validate.
    def Validate(self, Key, Value):
        # If key is in params AO (allowed options), then no need to check for any validity.
        if Key in self.ParamsAO:
            return True

        if not Key in self.Params:
            return False

        return Value in self.Params[Key]
