.zeropage

;internal bios area
z_tick: .res 1
z_cyc: .res 1

nmi_h: .res 2
irq_h: .res 2

z_arg:    .res 1
z_addr:   .res 2
z_result: .res 1

xe_entry: .res 2
xe_boot:  .res 1
fi_sect:  .res 1
fi_len:   .res 1
fi_addr:  .res 1
fi_name:  .res 8

;arguments
z_src:   .res 2
z_dest:  .res 2
z_count: .res 2

.segment "BSS"

w_disk_buf: .res 512