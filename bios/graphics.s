tileset_bios:
.incbin "tileset.chr", $00, $A0

tilemap_bios:
	.byte 1, 2, 0, 0, 0, 0, 1, 2
	.byte 3, 8, 4, 0, 0, 5, 8, 6
	.byte 0, 3, 8, 4, 5, 8, 6, 0
	.byte 0, 0, 3, 8, 8, 6, 0, 0
	.byte 0, 0, 5, 8, 8, 4, 0, 0
	.byte 0, 5, 8, 6, 3, 8, 4, 0
	.byte 5, 8, 6, 0, 0, 3, 8, 4
	.byte 7, 9, 0, 0, 0, 0, 7, 9