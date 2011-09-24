OUTPUT_FORMAT("binary")
ENTRY(main)

SECTIONS
{
    .header 0xD000 : AT(0xD000)
    {
        header = .;
        *(.header)
    }

    .text 0xD000 + (code - header) :
    {
        code = .; _code = .; __code = .;
        *(.text)
        *(.rodata)
    }

    .data 0xD000 + (data - header) :
    {
        data = .; _data = .; __data = .;
        *(.data)                                                                                                                                                                                                            
    }

    file_end = .; _file_end = .; __file_end = .;

    .bss 0xD000 + (bss - header) :
    {
        bss = .;
        *(.bss)
    }

    end = .; _end = .; __end = .;
}

