; Common definitions to get the Video Mode information list.
;
; Copyright (c) 2011 Shikhin Sethi
;
; This program is free software; you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation; either version 3 of the License, or
; (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License along
; with this program; if not, write to the Free Software Foundation, Inc.,
; 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

SECTION .text

; Performs a switch to a VGA mode, using the BIOS.
; @ax             The mode to switch to.
SwitchVGA:
    pushad

    int 0x10                          ; Since the mode is in AX, AH should be cleared. So, switch!

.Return:
    popad
    ret
