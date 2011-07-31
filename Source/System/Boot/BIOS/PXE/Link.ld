OUTPUT_FORMAT("binary")
ENTRY(Startup)

SECTIONS
{
    .text 0x7C00 : AT(0x7C00) 
    {
        code = .;
        *(.text)
        *(.rodata)
    }

    .data 0x7C00 + (data - code) :
    {
        data = .;
        *(.data)                                                                                                                                                 }

    end = .;
}

