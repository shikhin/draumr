OUTPUT_FORMAT("binary")
ENTRY(Startup)

SECTIONS
{
    .base 0x7C00 : AT(0x7C00)
    {
        base = .;
        *(.base)
    }

    .text 0x7C00 + (code - base) :
    {
        code = .;
        *(.text)
        *(.rodata)
    }

    .data 0x7C00 + (data - base) :
    {
        data = .;
        *(.data)                                                                                                                                                                                                            
    }

    .pad 0x8BF8 :
    {
        pad = .;
        *(.pad)
    }

    end = .;
}

