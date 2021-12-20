#include "common.hpp"

struct MultibootInfo {
	u32 flags;
	u32 mem_lower;
	u32 mem_upper;
	u32 boot_device;
	u32 cmdline;
	u32 mods_count;
	u32 mods_addr;
	u32 syms[4];
	u32 mmap_length;
	u32 mmap_addr;
	u32 drives_length;
	u32 drives_addr;
	u32 config_table;
	u32 boot_loader_name;
	u32 apm_table;

	u32 vbe_control_info;
	u32 vbe_mode_info;
	u16 vbe_mode;
	u16 vbe_interface_seg;
	u16 vbe_interface_off;
	u16 vbe_interface_len;

	u64 framebuffer_addr;
	u32 framebuffer_pitch;
	u32 framebuffer_width;
	u32 framebuffer_height;
	u8 framebuffer_bpp;
	u8 framebuffer_type;
	union {
		struct {
			u32 framebuffer_palette_addr;
			u32 framebuffer_palette_num_colors;
		} indexed;
		struct {
			u8 framebuffer_red_field_position;
			u8 framebuffer_red_mask_size;
			u8 framebuffer_green_field_position;
			u8 framebuffer_green_mask_size;
			u8 framebuffer_blue_field_position;
			u8 framebuffer_blue_mask_size;
		} rgb;
	} framebuffer_color;
};
