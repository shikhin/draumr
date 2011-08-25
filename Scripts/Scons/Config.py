# Draumr build system
#
# Copyright (c) 2011 Zoltan Kovacs, Shikhin Sethi
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

from SCons.Errors import StopError

class Config :
    params = {
        "build" : ["debug", "release"],
        "target" : ["iso", "pxe", "floppy", "all"],
    }

    def __init__(self) :
        self.arch = "i686"
        self.build = "debug"
        self.target = "iso"
	
    def get_arch(self) :
        return self.arch

    def get_build(self) :
        return self.build

    def parse(self, args) :
        for k in args.keys() :
            v = args[k]

            if not self._validate(k, v) :
                raise StopError("Invalid value for %s parameter. Allowed values: %s." % (k, ", ".join(self.params[k])))

            if k == "build" : self.build = v
            if k == "target" : self.target = v

    def _validate(self, key, value) :
        if not key in self.params :
            return False

        return value in self.params[key]
