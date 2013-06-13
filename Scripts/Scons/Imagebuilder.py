 # Draumr build system.
 #
 # Copyright (c) 2012, Shikhin Sethi
 # All rights reserved.
 #
 # Redistribution and use in Source and binary forms, with or without
 # modification, are permitted provided that the following conditions are met:
 #     * Redistributions of Source code must retain the above copyright
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

# Import required modules.
import os

from SCons.Builder import Builder
from SCons.Action import Action

def _image_builder(target, source, env):
    # Run the CRC32 utility on all targets.
    for Target in env["COMMON_TARGETS"]:
        os.system("%s %s" % (env["CRC32"][0], Target[0]))
    
    if os.path.isfile(env["BACK"]):
        # Run the ToSIF utility, and then the CRC32 utility.
        os.system("%s %s Background.sif" % (env["ToSIF"][0], env["BACK"]))
        env["BACK"] = "Background.sif"              
        os.system("%s %s" % (env["CRC32"][0], env["BACK"]))
        
    else:        
        env["BACK"] = '\0x00'
                 
    return 0

ImageBuilder = Builder(action = Action(_image_builder, None))

