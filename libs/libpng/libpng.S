	.set noreorder

#include "pspimport.s"

// Build files
// libpng_XXXX.o

#ifdef F_libpng_0000
	IMPORT_START "libpng",0x40090000
#endif
#ifdef F_libpng_0001
	IMPORT_FUNC  "libpng",0x596C9364,png_get_libpng_ver
#endif
#ifdef F_libpng_0002
	IMPORT_FUNC  "libpng",0x9785BA9A,png_access_version_number
#endif
#ifdef F_libpng_0003
	IMPORT_FUNC  "libpng",0x1A1F232F,png_set_sig_bytes
#endif
#ifdef F_libpng_0004
	IMPORT_FUNC  "libpng",0xD4E37F9F,png_sig_cmp
#endif
#ifdef F_libpng_0005
	IMPORT_FUNC  "libpng",0x6528115D,png_create_read_struct
#endif
#ifdef F_libpng_0006
	IMPORT_FUNC  "libpng",0x4644B335,png_create_write_struct
#endif
#ifdef F_libpng_0007
	IMPORT_FUNC  "libpng",0x806D225C,png_get_compression_buffer_size
#endif
#ifdef F_libpng_0008
	IMPORT_FUNC  "libpng",0x42001996,png_set_compression_buffer_size
#endif
#ifdef F_libpng_0010
	IMPORT_FUNC  "libpng",0xC2CF0FF7,png_set_longjmp_fn
#endif
#ifdef F_libpng_0011
	IMPORT_FUNC  "libpng",0x0C8288B5,png_write_sig
#endif
#ifdef F_libpng_0012
	IMPORT_FUNC  "libpng",0x6F3BFE3A,png_write_chunk
#endif
#ifdef F_libpng_0013
	IMPORT_FUNC  "libpng",0xE0C30F0B,png_write_chunk_start
#endif
#ifdef F_libpng_0014
	IMPORT_FUNC  "libpng",0xA3581C2A,png_write_chunk_data
#endif
#ifdef F_libpng_0015
	IMPORT_FUNC  "libpng",0x9A7A25D6,png_write_chunk_end
#endif
#ifdef F_libpng_0016
	IMPORT_FUNC  "libpng",0x7AFD6B74,png_create_info_struct
#endif
#ifdef F_libpng_0017
	IMPORT_FUNC  "libpng",0x9CF260A4,png_info_init_3
#endif
#ifdef F_libpng_0018
	IMPORT_FUNC  "libpng",0x769D7C3E,png_write_info_before_PLTE
#endif
#ifdef F_libpng_0019
	IMPORT_FUNC  "libpng",0xD967E431,png_write_info
#endif
#ifdef F_libpng_0020
	IMPORT_FUNC  "libpng",0xE327C974,png_read_info
#endif
#ifdef F_libpng_0021
	IMPORT_FUNC  "libpng",0xDB1549DF,png_start_read_image
#endif
#ifdef F_libpng_0022
	IMPORT_FUNC  "libpng",0xD4F9270D,png_read_update_info
#endif
#ifdef F_libpng_0023
	IMPORT_FUNC  "libpng",0x03FF4CFC,png_write_row
#endif
#ifdef F_libpng_0024
	IMPORT_FUNC  "libpng",0x753C7F04,png_write_rows
#endif
#ifdef F_libpng_0025
	IMPORT_FUNC  "libpng",0x7BC7020C,png_write_image
#endif
#ifdef F_libpng_0026
	IMPORT_FUNC  "libpng",0xB0A3C492,png_write_end
#endif
#ifdef F_libpng_0027
	IMPORT_FUNC  "libpng",0xB4EADCDD,png_destroy_info_struct
#endif
#ifdef F_libpng_0028
	IMPORT_FUNC  "libpng",0x211A764A,png_destroy_read_struct
#endif
#ifdef F_libpng_0029
	IMPORT_FUNC  "libpng",0x2CB9F5BC,png_destroy_write_struct
#endif
#ifdef F_libpng_0030
	IMPORT_FUNC  "libpng",0xF71634C8,png_set_crc_action
#endif
#ifdef F_libpng_0031
	IMPORT_FUNC  "libpng",0x25B86FCC,png_set_filter
#endif
#ifdef F_libpng_0032
	IMPORT_FUNC  "libpng",0xDC830C64,png_set_compression_level
#endif
#ifdef F_libpng_0033
	IMPORT_FUNC  "libpng",0x97273790,png_set_compression_mem_level
#endif
#ifdef F_libpng_0034
	IMPORT_FUNC  "libpng",0x45B94A93,png_set_compression_strategy
#endif
#ifdef F_libpng_0035
	IMPORT_FUNC  "libpng",0x05B97898,png_set_compression_window_bits
#endif
#ifdef F_libpng_0036
	IMPORT_FUNC  "libpng",0x96FEBB16,png_set_compression_method
#endif
#ifdef F_libpng_0037
	IMPORT_FUNC  "libpng",0xF26EE1D8,png_set_error_fn
#endif
#ifdef F_libpng_0038
	IMPORT_FUNC  "libpng",0x16C92089,png_get_error_ptr
#endif
#ifdef F_libpng_0039
	IMPORT_FUNC  "libpng",0x86D1D464,png_set_write_fn
#endif
#ifdef F_libpng_0040
	IMPORT_FUNC  "libpng",0x1BDFF258,png_set_read_fn
#endif
#ifdef F_libpng_0041
	IMPORT_FUNC  "libpng",0xEE1C42F5,png_get_io_ptr
#endif
#ifdef F_libpng_0042
	IMPORT_FUNC  "libpng",0xE65B8CCD,png_set_read_status_fn
#endif
#ifdef F_libpng_0043
	IMPORT_FUNC  "libpng",0x3E479676,png_set_write_status_fn
#endif
#ifdef F_libpng_0044
	IMPORT_FUNC  "libpng",0x1482C41A,png_warning
#endif
#ifdef F_libpng_0045
	IMPORT_FUNC  "libpng",0xC0998546,png_chunk_warning
#endif
#ifdef F_libpng_0046
	IMPORT_FUNC  "libpng",0x966E245A,png_get_valid
#endif
#ifdef F_libpng_0047
	IMPORT_FUNC  "libpng",0x34FF569F,png_get_rowbytes
#endif
#ifdef F_libpng_0048
	IMPORT_FUNC  "libpng",0x2CF371C7,png_get_rows
#endif
#ifdef F_libpng_0049
	IMPORT_FUNC  "libpng",0xDAB724F8,png_set_rows
#endif
#ifdef F_libpng_0050
	IMPORT_FUNC  "libpng",0x67EF4D65,png_get_channels
#endif
#ifdef F_libpng_0051
	IMPORT_FUNC  "libpng",0xE6D00C8B,png_get_image_width
#endif
#ifdef F_libpng_0052
	IMPORT_FUNC  "libpng",0x518922E5,png_get_image_height
#endif
#ifdef F_libpng_0053
	IMPORT_FUNC  "libpng",0x8F07ABC9,png_get_bit_depth
#endif
#ifdef F_libpng_0054
	IMPORT_FUNC  "libpng",0xDB5BFE20,png_get_color_type
#endif
#ifdef F_libpng_0055
	IMPORT_FUNC  "libpng",0xAA1EE10F,png_get_filter_type
#endif
#ifdef F_libpng_0056
	IMPORT_FUNC  "libpng",0xFB6AD168,png_get_interlace_type
#endif
#ifdef F_libpng_0057
	IMPORT_FUNC  "libpng",0x9C03DD98,png_get_compression_type
#endif
#ifdef F_libpng_0058
	IMPORT_FUNC  "libpng",0x49ABC215,png_get_pixels_per_meter
#endif
#ifdef F_libpng_0059
	IMPORT_FUNC  "libpng",0xA1420092,png_get_x_pixels_per_meter
#endif
#ifdef F_libpng_0060
	IMPORT_FUNC  "libpng",0x888485BC,png_get_y_pixels_per_meter
#endif
#ifdef F_libpng_0061
	IMPORT_FUNC  "libpng",0x78BF1620,png_get_x_offset_pixels
#endif
#ifdef F_libpng_0062
	IMPORT_FUNC  "libpng",0xB6CD1937,png_get_y_offset_pixels
#endif
#ifdef F_libpng_0063
	IMPORT_FUNC  "libpng",0xC508FBAE,png_get_x_offset_microns
#endif
#ifdef F_libpng_0064
	IMPORT_FUNC  "libpng",0x832391D8,png_get_y_offset_microns
#endif
#ifdef F_libpng_0065
	IMPORT_FUNC  "libpng",0x77D426FA,png_get_signature
#endif
#ifdef F_libpng_0066
	IMPORT_FUNC  "libpng",0x38F12307,png_get_IHDR
#endif
#ifdef F_libpng_0067
	IMPORT_FUNC  "libpng",0xDDD91620,png_read_image
#endif
#ifdef F_libpng_0068
	IMPORT_FUNC  "libpng",0x56488D04,png_set_filler
#endif
#ifdef F_libpng_0069
	IMPORT_FUNC  "libpng",0x5DBA44FE,png_set_expand
#endif
#ifdef F_libpng_0070
	IMPORT_FUNC  "libpng",0x26F5F341,png_set_strip_16
#endif
#ifdef F_libpng_0071
	IMPORT_FUNC  "libpng",0x5DBA44FE,png_set_expand
#endif
#ifdef F_libpng_0072
	IMPORT_FUNC  "libpng",0x7333E75A,png_set_packing
#endif
#ifdef F_libpng_0073
	IMPORT_FUNC  "libpng",0x2DA09328,png_set_tRNS_to_alpha
#endif
#ifdef F_libpng_0074
	IMPORT_FUNC  "libpng",0x5DBA44FE,png_set_expand
#endif
#ifdef F_libpng_0075
	IMPORT_FUNC  "libpng",0x3A230E18,png_set_expand_gray_1_2_4_to_8
#endif
#ifdef F_libpng_0076
	IMPORT_FUNC  "libpng",0x4C597BB0,png_set_palette_to_rgb
#endif
#ifdef F_libpng_0077
	IMPORT_FUNC  "libpng",0x1C5575D5,png_init_io
#endif
#ifdef F_libpng_0078
	IMPORT_FUNC  "libpng",0xA95E38D3,png_set_IHDR
#endif
#ifdef F_libpng_0079
	IMPORT_FUNC  "libpng",0xAAB30565,png_read_row
#endif
#ifdef F_libpng_0080
	IMPORT_FUNC  "libpng",0xB2290D1A,png_read_end
#endif
