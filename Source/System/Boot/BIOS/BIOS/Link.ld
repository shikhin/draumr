OUTPUT_FORMAT("binary")
ENTRY(Startup)

SECTIONS
{
    .header 0x9000 : AT(0x9000)
    {
        header = .;
        *(.header)
    }

    .text 0x9000 + (code - header) :
    {
        code = .;
        *(.text)
        *(.rodata)
    }

    .data 0x9000 + (data - header) :
    {
        data = .;
        *(.data)                                                                                                                                                 }

    .bss 0x9000 + (bss - header) :
    {
        bss = .;
        *(.bss)
    }
    end = .;

    .pad 0xAFFC :
    {
        pad = .;
        *(.pad)
    }

    file_end = .;
}

