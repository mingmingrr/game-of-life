#include <ctime>
#include <iomanip>
#include <filesystem>
#include <fstream>
#include <ostream>
#include <chrono>
#include <cstdlib>
#include <string>
#include <sul/dynamic_bitset.hpp>
#include <ncursesw/curses.h>

struct tracetime {};
struct tracetime now;

std::wostream& operator <<(std::wostream& output, const struct tracetime& _) {
	std::time_t t = std::time(nullptr);
	output << std::put_time(std::localtime(&t), L"[%H:%M:%S.")
		<< std::setw(3) << std::setfill(L'0')
		<< std::chrono::duration_cast<std::chrono::milliseconds>
			(std::chrono::system_clock::now().time_since_epoch()).count() % 1000
		<< ']';
	return output;
}

struct tracemod {};
struct tracemod mod;

void tracef(std::wostream& out) {
	out << std::endl;
}

template<typename X, typename ...Xs>
void tracef(std::wostream& out, struct tracemod _, X&& x, Xs&& ...xs) {
	out << x;
	return tracef(out, xs...);
}

template<typename X, typename ...Xs>
void tracef(std::wostream& out, X&& x, Xs&& ...xs) {
	out << x << ' ';
	return tracef(out, xs...);
}

template<typename T>
decltype(auto) identity(T&& t) {
	return std::forward<T>(t);
}

template<typename ...Xs>
decltype(auto) trace(Xs&& ...xs) {
// #ifndef NDEBUG
	auto fifo = std::filesystem::path("debug.fifo");
	if(std::filesystem::exists(fifo)) {
		std::wofstream file(fifo);
		tracef(file, xs...);
	}
// #endif
	return (identity(std::forward<Xs>(xs)), ...);
}


int main() {
	trace(now, "========================================");

	srand(time(NULL));
	WINDOW* win = initscr();
	cbreak();
	curs_set(0);
	mousemask(ALL_MOUSE_EVENTS, NULL);
	// nodelay(win, true);

	static wchar_t blocks[] = L" ▀▄█";
	const int lines = getmaxy(win), cols = getmaxx(win);

	const int inner_rows = 2 * lines, inner_cols = cols;
	const int outer_rows = inner_rows + 2, outer_cols = inner_cols + 2;
	const int outer_squares = outer_rows * outer_cols;
	const int inner_squares = inner_rows * outer_cols;

	sul::dynamic_bitset<> outer_top(outer_squares);
	outer_top.flip(0, outer_cols);
	sul::dynamic_bitset<> inner_top(outer_squares);
	inner_top.flip(outer_cols, outer_cols);
	sul::dynamic_bitset<> inner_bot(outer_squares);
	inner_bot.flip(inner_squares, outer_cols);
	sul::dynamic_bitset<> outer_bot(outer_squares);
	outer_bot.flip(outer_squares - outer_cols, outer_cols);

	sul::dynamic_bitset<> outer_left(outer_squares);
	for(int i = 0; i < outer_squares; i += outer_cols)
		outer_left.set(i);
	sul::dynamic_bitset<> inner_left = outer_left << 1;
	sul::dynamic_bitset<> inner_right = outer_left << inner_cols;
	sul::dynamic_bitset<> outer_right = inner_right  << 1;

	sul::dynamic_bitset<> outer_mask
		= outer_top | outer_bot | outer_left | outer_right;
	sul::dynamic_bitset<> inner_mask = ~outer_mask;

	sul::dynamic_bitset<> grid(outer_squares);
	for(int i = 0; i < outer_squares; ++i)
		grid.set(i, rand() % 4 == 0);

	trace(now, grid);

	clear();
	trace(now, lines, inner_cols);
	for(int r = 0; r < lines; ++r) {
		std::wstring line(inner_cols, '-');
		for(int c = 0; c < inner_cols; ++c)
			line[c] = blocks[grid[(2 * r + 1) * outer_cols + c + 1]
				+ (grid[(2 * r + 2) * outer_cols + c + 1] << 1)];
		trace(now, "line:");
		trace('>', line, '<');
		trace("lineend");
		mvins_nwstr(r, 0, line.data(), inner_cols);
	}

	getch();

/*
	= random.randint(0, grid_mask)
	grid &= random.randint(0, grid_mask)
	stdscr.clear()
	for r in range(lines):
		l = grid >> ((2 * r + 1) * outer_cols + 1)
		stdscr.insstr(r, 0, ''.join(
			blocks[((l >> c) & 1) | ((l >> (c + outer_cols - 1)) & 2)]
			for c in range(cols)))

	generate = 0

	while stdscr.getch() == -1:
		time.sleep(0.05)

		grid &= inner_mask
		grid |= ((grid >> inner_cols) & outer_left) | ((grid << inner_cols) & outer_right)
		grid |= ((grid >> inner_squares) & outer_top) | ((grid << inner_squares) & outer_bot)

		checks = grid | (grid << 1) | (grid >> 1)
		checks |= (grid << outer_cols) | (grid >> outer_cols)
		checks &= inner_mask

		life = grid

		while checks:
			bit = checks & -checks
			checks &= ~bit

			neigh = bit | (bit << 1) | (bit >> 1)
			neigh |= (neigh << outer_cols) | (neigh >> outer_cols)
			neigh = (neigh & grid).bit_count()

			if (grid & bit):
				if not (3 <= neigh <= 4):
					life &= ~bit
			elif neigh == 3:
					life |= bit

		grid = life

		generate += 1
		if generate == 10:
			generate, times = 0, round(outer_rows * outer_cols
				/ max(1, grid.bit_count()) / 10)
			for _ in range(times):
				grid |= 1 << random.randint(0, outer_rows * outer_cols - 1)

		for r in range(lines):
			l = grid >> ((2 * r + 1) * outer_cols + 1)
			stdscr.insstr(r, 0, ''.join(
				blocks[((l >> c) & 1) | ((l >> (c + outer_cols - 1)) & 2)]
				for c in range(cols)))

curses.wrapper(main)
*/
}

