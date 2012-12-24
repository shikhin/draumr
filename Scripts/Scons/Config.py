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
import ConfigParser

from SCons.Errors import StopError

class Config:
    # The expected (and parsable) parameters with options.
    Params = {
        "build": ["debug", "release"],
        "target": ["iso", "pxe", "floppy", "all"],
        "arch": ["x86_64", "i586"]
    }

    # The expected parameters with any option parsable.
    ParamsAO = {
        "pxepath"
    }

    def __init__(self):
        # The default values.
        self.Arch = "x86_64"
        self.Build = "debug"
        self.Target = "all"
        self.PXEPath = "/tftpboot"

        # The configuration file location.
        self.ConfigFile = "Build.cfg"

    def Parse(self, Args):
        # Get the config file name, so that we can parse it before parsing the options.
        for Key in Args.keys():
            if Key == "configfile": self.ConfigFile = Args[Key]

        ConfigFile = ConfigParser.SafeConfigParser()
        ConfigFile.read(self.ConfigFile)

        #if(ConfigFile.has_option('', ''))

        for Key in Args.keys():
            # We have already looked at the config file option.
            if Key == "configfile": continue

            # Get the value.
            Value = Args[Key]

            # See if the value is parsable or not.
            if not self.Validate(Key, Value):
                raise StopError("Invalid value for %s parameter. Allowed values: %s." % (Key, ", ".join(self.Params[Key])))

            # Set the new values.
            if Key == "build": self.Build = Value
            if Key == "target": self.Target = Value
            if Key == "pxepath": self.PXEPath = Value
            if Key == "arch": self.Arch = Value

    # Validate.
    def Validate(self, Key, Value):
        # If key is in params AO (allowed options), then no need to check for any validity.
        if Key in self.ParamsAO:
            return True

        if not Key in self.Params:
            return False

        return Value in self.Params[Key]
