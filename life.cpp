#include <ctime>
#include <iomanip>
#include <filesystem>
#include <fstream>
#include <ostream>
#include <chrono>
#include <cstdlib>
#include <thread>
#include <string>
#include <iostream>
#include <sul/dynamic_bitset.hpp>
#include <ncursesw/curses.h>

using namespace std::chrono_literals;

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

struct CursesWin {
	CursesWin() {
		initscr();
		cbreak();
		noecho();
		curs_set(false);
		nodelay(stdscr, true);
		keypad(stdscr, true);
		mousemask(ALL_MOUSE_EVENTS, NULL);
	}
	~CursesWin() {
		keypad(stdscr, false);
		echo();
		nocbreak();
		endwin();
	}
};

int main() {
	trace(now, "========================================");
	std::locale::global(std::locale(""));

	srand(time(NULL));
	CursesWin _cwin;

	const wchar_t blocks[] = L" ▀▄█";
	const int lines = getmaxy(stdscr), cols = getmaxx(stdscr);

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

	sul::dynamic_bitset<> neigh_mask(outer_squares);
	for(int i = 0; i < 3; ++i)
		neigh_mask.set(outer_cols * i, 3, true);

	sul::dynamic_bitset<> grid(outer_squares);
	for(int i = 0; i < outer_squares; ++i)
		grid.set(i, rand() % 4 == 0);

	clear();

	while(getch() == -1) {
		for(int r = 0; r < lines; ++r) {
			std::wstring line(inner_cols, '-');
			for(int c = 0; c < inner_cols; ++c)
				line[c] = (int)blocks[grid[(2 * r + 1) * outer_cols + c + 1]
					+ (grid[(2 * r + 2) * outer_cols + c + 1] << 1)];
			mvins_nwstr(r, 0, line.data(), inner_cols);
		}

		std::this_thread::sleep_for(100ms);

		grid &= inner_mask;
		grid |= ((grid >> inner_cols) & outer_left)
			| ((grid << inner_cols) & outer_right);
		grid |= ((grid >> inner_squares) & outer_top)
			| ((grid << inner_squares) & outer_bot);

		sul::dynamic_bitset<> checks = grid | (grid << 1) | (grid >> 1);
		checks |= (grid << outer_cols) | (grid >> outer_cols);
		checks &= inner_mask;

		sul::dynamic_bitset<> threes(outer_squares);
		sul::dynamic_bitset<> fours(outer_squares);
		checks.iterate_bits_on([&](int bit) {
			int count = (grid & (neigh_mask << (bit - 1 - outer_cols))).count();
			switch(count) {
				case 3: threes.set(bit); break;
				case 4: fours.set(bit); break;
			}
		});

		grid = (grid & (threes | fours)) | (~grid & threes);

/*
		generation += 1
		if generation == 10:
			generation, times = 0, round(outer_rows * outer_cols
				/ max(1, grid.bit_count()) / 10)
			for _ in range(times):
				grid |= 1 << random.randint(0, outer_rows * outer_cols - 1)
*/
	}
}

