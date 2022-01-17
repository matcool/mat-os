#pragma once
#include "common.hpp"
#include <lib/vector.hpp>
#include <lib/pointer.hpp>
#include <lib/math.hpp>

class Window;

struct Rect {
	Vec2<u32> pos, size;
};

class Screen {
public:
	u32 width, height;
	u32* buffer_a;
	u32* buffer_b;
	Vector<OwnPtr<Window>> m_windows;
	Vector<Rect> m_update_rects;

	static auto& get() {
		static Screen instance;
		return instance;
	}

	// TODO: move this to the ctor somehow
	void init(u32 width, u32 height, u32*);

	void redraw();
	void clear();
	void swap();
	void refresh();

	// color is in the format 0xAARRGGB, although
	// the alpha makes no sense in the context of a screen
	inline void set_pixel(u32 x, u32 y, u32 color) {
		if (x >= width || y >= height) return;
		buffer_b[y * width + x] = color;
	}
};

class Window {
protected:
	// TODO: vec structs
	Vec2<u32> m_position, m_prev_position;
	Vec2<u32> m_size;
	friend Screen;
	bool m_has_refreshed_move = false;
public:
	Vector<u32> m_buffer;
	virtual ~Window() = default;

	void set_position(u32 x, u32 y) {
		if (!m_has_refreshed_move) {
			m_has_refreshed_move = true;
			update_entire_thing_lol();
		}
		m_position = { x, y };
	}

	Window(u32 x, u32 y, u32 width, u32 height)
		: m_position(x, y), m_prev_position(m_position), m_size(width, height) {
		m_buffer.reserve(height * width);
		for (size_t i = 0; i < width * height; ++i)
			m_buffer.push_back(0);
	}

	void update_screen(const Vec2<u32> pos, const Vec2<u32> size);

	void update_entire_thing_lol();

	auto size() const { return m_size; }
	inline auto& buffer() { return m_buffer; }

	inline void set_pixel(u32 x, u32 y, u32 color) {
		if (x >= m_size.width || y >= m_size.height) return;
		m_buffer[y * m_size.width + x] = color;
	}
};
