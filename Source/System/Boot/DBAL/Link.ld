OUTPUT_FORMAT("binary")
ENTRY(Start)

SECTIONS
{
    .header 0xE000 :
    {
        header = .;
        *(.header)
    }

    .text :
    {
        code = .; _code = .; __code = .;
        *(.text)
        *(.rodata)
    }

    .data :
    {
        data = .; _data = .; __data = .;
        *(.data)                                                                                                                                                                                                            
    }

    file_end = .; _file_end = .; __file_end = .;

    .bss :
    {
        bss = .;
        *(.bss)
    }

    end = .; _end = .; __end = .;
}

