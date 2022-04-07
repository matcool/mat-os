#ifndef MAT_OS
#include <string.h>
#include <iostream>
#include <new>
#else
#include "common.hpp"
#endif

#include <lib/string.hpp>
#include <lib/format.hpp>
#include <lib/vector.hpp>
#include <lib/function.hpp>
#include <lib/utils.hpp>
#include <lib/tuple.hpp>
#include <lib/optional.hpp>
#include <lib/string-utils.hpp>
#include <lib/result.hpp>

template <class... Args>
void print(Args&&... args) {
#ifndef MAT_OS
	format_to([](char c) { std::cout.put(c); }, args...);
#endif
}

enum class Register {
	eax,
	ecx,
	edx,
	ebx,
	esp,
	ebp,
	esi,
	edi,

	ax,
	cx,
	dx,
	bx,
	sp,
	bp,
	si,
	di
};

Optional<Register> register_from_name(const StringView str) {
	if (str == "eax"_sv) return Register::eax;
	if (str == "ecx"_sv) return Register::ecx;
	if (str == "edx"_sv) return Register::edx;
	if (str == "ebx"_sv) return Register::ebx;
	if (str == "esp"_sv) return Register::esp;
	if (str == "ebp"_sv) return Register::ebp;
	if (str == "esi"_sv) return Register::esi;
	if (str == "edi"_sv) return Register::edi;
	return {};
}

struct Operand {
	enum class Type {
		REGISTER,
		IMMEDIATE,
		MODRM // stuff like [eax], [eax + ebx], [eax + 0x10], [eax + ecx * 2 + 0x10]
	} type;
	u32 value{};
	Register reg{};
	// for modrm
	u8 mode{32};
	Register scaled_reg{};
	u8 scale{};
	bool has_base_reg{};
};

StringView enum_name(const Operand::Type value) {
	switch (value) {
		case Operand::Type::REGISTER: return "REGISTER"_sv;
		case Operand::Type::IMMEDIATE: return "IMMEDIATE"_sv;
		case Operand::Type::MODRM: return "MODRM"_sv;
	}
}

StringView enum_name(const Register value) {
	switch (value) {
		case Register::eax: return "eax"_sv;
		case Register::ecx: return "ecx"_sv;
		case Register::edx: return "edx"_sv;
		case Register::ebx: return "ebx"_sv;
		case Register::esp: return "esp"_sv;
		case Register::ebp: return "ebp"_sv;
		case Register::esi: return "esi"_sv;
		case Register::edi: return "edi"_sv;
		default: return "idk"_sv;
	}
}

template <class T>
requires requires (const T a) { enum_name(a); }
struct Formatter<T> {
	static void format(auto write, const T value, const StringView& opt) {
		Formatter<StringView>::format(write, enum_name(value), opt);
	}
};

template <>
struct Formatter<Operand> {
	static void format(auto write, const Operand& op, const StringView&) {
		format_to(write, R"(Operand {{
  type = {},
  value = {},
  reg = {},
  mode = {},
  off_reg = {},
  scale = {},
  has_base_reg = {}
}})", op.type, op.value, op.reg, op.mode, op.scaled_reg, op.scale, op.has_base_reg);
	}
};

// TODO: add this to lib
bool is_numeric(const char ch) {
	return ch >= '0' && ch <= '9';
}

Result<Operand> parse_operand(const StringView str) {
	const auto first = str[0];
	// TODO: dword ptr and shit
	if (first == '[') {
		const auto end = str.find(']');
		Operand op;
		op.type = Operand::Type::MODRM;
		bool found_reg{}, found_off_reg{}, found_off{};
		for (auto bit : str.sub(1, end).split_iter('+')) {
			bit = bit.trim();
			if (is_numeric(bit[0])) {
				if (found_off) {
					// TODO: make the error an enum?
					return make_error("two offsets"_sv);
				}
				found_off = true;
				op.value = parse_int_literal<u32>(bit);
			} else {
				const auto star = bit.find('*');
				auto name = bit;
				bool is_scale = false;
				// TODO: eax + esp and esp + eax both result in esp + eax * 1
				if (star != size_t(-1)) {
					if (found_off_reg) {
						return make_error("two offset regs"_sv);
					}
					found_off_reg = true;
					is_scale = true;
					name = bit.sub(0, star).trim_right();
					op.scale = parse_int<u8>(bit.sub(star + 1).trim_left(), 10);
				} else {
					if (found_reg) {
						if (!found_off_reg) {
							found_off_reg = true;
							op.scale = 1;
							is_scale = true;
						} else {
							return make_error("two regs"_sv);
						}
					}
					found_reg = true;
					op.has_base_reg = true;
				}
				auto reg = register_from_name(name);
				// TODO: cant scale esp
				if (!reg)
					return make_error("invalid register"_sv);
				(is_scale ? op.scaled_reg : op.reg) = *reg;
			}
		}
		return op;
	} else if (is_numeric(first)) {
		Operand op;
		op.type = Operand::Type::IMMEDIATE;
		op.value = parse_int_literal<u32>(str);
		return op;
	} else {
		// either register or label
		// labels not supported yet
		Operand op;
		op.type = Operand::Type::REGISTER;
		const auto reg = register_from_name(str);
		if (!reg)
			return make_error("invalid register"_sv);
		op.reg = *reg;
		return op;
	}
}

template <class T>
concept byte_vector = requires (T& vec, u8 a) { vec.push_back(a); };

template <integral T, byte_vector Vec>
void push_le_int(Vec& vec, const T value) {
	vec.push_back(value & 0xFF);
	if constexpr (sizeof(T) >= 2) {
		vec.push_back((value >> 8) & 0xFF);
	}
	if constexpr (sizeof(T) >= 4) {
		vec.push_back((value >> 16) & 0xFF);
		vec.push_back((value >> 24) & 0xFF);
	}
}

constexpr u8 index_for_reg(const Register reg) {
	return static_cast<u8>(reg) - static_cast<u8>(Register::eax);
}

template <byte_vector Vec>
Result<void> encode_modrm(Vec& vec, const Operand& op, const u8 digit) {
	if (op.type == Operand::Type::REGISTER) {
		const auto index = index_for_reg(op.reg);
		vec.push_back(index | (digit << 3) | (0b11 << 6));
		return {};
	}
	// TODO: check if base reg is not esp
	if (op.scale == 0) {
		// TODO: not always use 32 bit
		const u8 mod = (op.value || op.reg == Register::ebp) ? 0b10 : 0b00;
		const u8 rm = index_for_reg(op.reg);
		vec.push_back(rm | (digit << 3) | (mod << 6));
		if (mod)
			push_le_int<u32>(vec, op.value);
	} else {
		const u8 mod = op.value ? 0b10 : 0b00;
		const u8 rm = 4; // sib
		vec.push_back(rm | (digit << 3) | (mod << 6));
		u8 ss{};
		// its just log2
		switch (op.scale) {
			case 1: ss = 0b00; break;
			case 2: ss = 0b01; break;
			case 4: ss = 0b10; break;
			case 8: ss = 0b11; break;
			default: {
				return make_error("invalid scale"_sv);
			}
		}
		const auto base = op.has_base_reg ? index_for_reg(op.reg) : 5;
		const auto index = index_for_reg(op.scaled_reg);
		vec.push_back(base | (index << 3) | (ss << 6));
		// base=5 forces the disp32 [*]
		if (mod || base == 5)
			push_le_int<u32>(vec, op.value);
	}
	return {};
}

Result<Vector<u8>> assemble(const StringView& src) {
	Vector<u8> output;

	for (auto line : src.split_iter('\n')) {
		line = line.trim();
		if (!line) continue;
		if (line.starts_with("//"_sv)) continue;
		const auto size = output.size();
		const auto [name, params] = line.split_once(' ');
		// TODO: tolower them
		if (name == "nop"_sv) {
			const auto p = params.trim();
			if (p) {
				const auto op = parse_operand(p);
				if (!op) return make_error(op.error());
				output.push_back(0x0F);
				output.push_back(0x1F);
				const auto result = encode_modrm(output, op.ok(), 0);
				if (!result) return make_error(result.error());
			} else {
				output.push_back(0x90);
			}
		} else if (name == "hlt"_sv) {
			output.push_back(0xF4);
		} else if (name == "push"_sv) {
			const auto op_result = parse_operand(params.trim());
			if (!op_result) return make_error(op_result.error());
			const auto op = op_result.ok();
			if (op.type == Operand::Type::REGISTER) {
				output.push_back(0x50 + u8(op.reg) - u8(Register::eax));
			} else if (op.type == Operand::Type::IMMEDIATE) {
				// TODO: imm16 and imm8
				output.push_back(0x68);
				push_le_int<u32>(output, op.value);
			} else {
				output.push_back(0xFF);
				const auto result = encode_modrm(output, op, 6);
				if (!result) return make_error(result.error());
			}
		} else if (name == "pop"_sv) {
			const auto op_result = parse_operand(params.trim());
			if (!op_result) {
				print("parsing operand failed with {}\n"_sv, op_result.error());
				continue;
			}
			const auto op = op_result.ok();
			if (op.type == Operand::Type::REGISTER) {
				output.push_back(0x58 + u8(op.reg) - u8(Register::eax));
			} else if (op.type == Operand::Type::IMMEDIATE) {
				// assert false
			} else {
				output.push_back(0x8F);
				const auto result = encode_modrm(output, op, 0);
				if (!result) return make_error(result.error());
			}
		} else if (name == "ret"_sv) {
			output.push_back(0xC3);
		} else if (name == "mov"_sv) {
			auto [dst_str, src_str] = params.trim().split_once(',');
			const auto r_dst = parse_operand(dst_str.trim_right());
			const auto r_src = parse_operand(src_str.trim_left());
			if (!r_dst) return make_error(r_dst.error());
			if (!r_src) return make_error(r_src.error());
			const auto dst = r_dst.ok();
			const auto src = r_src.ok();
			if (dst.type == Operand::Type::IMMEDIATE) {
				return make_error("invalid operand to mov"_sv);
			}
			if (src.type == Operand::Type::IMMEDIATE) {
				if (dst.type == Operand::Type::REGISTER) {
					output.push_back(0xB8 + index_for_reg(src.reg));
				} else {
					output.push_back(0xC7);
					const auto result = encode_modrm(output, dst, 0);
					if (!result) return make_error(result.error());
				}
				push_le_int<u32>(output, src.value);
			} else if (dst.type == Operand::Type::MODRM) {
				if (dst.type == src.type)
					return make_error("invalid operands to mov"_sv);
				output.push_back(0x89);
				// src.type should be REGISTER
				const auto result = encode_modrm(output, dst, index_for_reg(src.reg));
				if (!result) return make_error(result.error());
			} else {
				output.push_back(0x8B);
				const auto result = encode_modrm(output, src, index_for_reg(dst.reg));
				if (!result) return make_error(result.error());
			}
		} else {
			if (name.trim())
				print("unhandled instruction `{}`\n", name);
			continue;
		}
		// pretty stuff
		print("\x1b[35;1m{}\x1b[0m {} ", name, params);
		for (size_t i = 0; i < 40 - (name.size() + params.size() + 1); ++i)
			print(" "_sv);
		for (size_t i = size; i < output.size(); ++i) {
			print("{:02X} ", output[i]);
		}
		print("\n");
	}
	return output;
}

int main() {
	const StringView src = R"(

mov eax, 69
ret

	)"_sv;

	const auto result = assemble(src);
	if (!result) {
		print("assembling failed: {}\n", result.error());
		return 1;
	}

	print("\noutput is: ");
	for (auto v : result.ok()) {
		print("{:02X} ", v);
	}
	print("\n");

	return 0;
}
