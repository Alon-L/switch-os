SECTIONS
{
  . = 0x1000;
  ._start : { *(._start) }
  .text : { *(.text) } 
}
