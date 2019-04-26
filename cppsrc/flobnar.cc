#include <assert.h>
#include <stdexcept>
#include <stdarg.h>
#include <stdio.h>
#include <string>
#include <vector>

static std::string format(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    char buffer[100];
    int len_without_nul = vsnprintf(buffer, sizeof buffer, fmt, ap);
    std::string result;
    if (len_without_nul < int(sizeof buffer)) {
        result = buffer;
    } else {
        va_end(ap);
        va_start(ap, fmt);
        result.resize(len_without_nul + 1);
        int n = vsnprintf(&result[0], result.size(), fmt, ap);
        assert(n == len_without_nul);
        result.resize(n);
        return result;
    }
    va_end(ap);
    return result;
}

enum class Dir : int {
    NORTH, SOUTH, EAST, WEST
};

struct Position {
    int row;
    int column;
    explicit Position(int r, int c) : row(r), column(c) {}
    Position min_with(int r, int c) const {
        return Position(std::min(row, r), std::min(column, c));
    }
    Position max_with(int r, int c) const {
        return Position(std::max(row, r), std::max(column, c));
    }
};

Position operator+(Position p, Dir step) {
    switch (step) {
        case Dir::NORTH: return Position(p.row-1, p.column);
        case Dir::SOUTH: return Position(p.row+1, p.column);
        case Dir::EAST: return Position(p.row, p.column+1);
        case Dir::WEST: return Position(p.row, p.column-1);
    }
}

void operator+=(Position& p, Dir step) {
    p = (p + step);
}

Dir flip(Dir step) {
    switch (step) {
        case Dir::NORTH: return Dir::SOUTH;
        case Dir::SOUTH: return Dir::NORTH;
        case Dir::EAST: return Dir::WEST;
        case Dir::WEST: return Dir::EAST;
    }
}

class Arguments {
    std::vector<int> data_;
public:
    explicit Arguments() = default;
    int head() const {
        if (data_.empty()) {
            return 0;
        } else {
            return data_.back();
        }
    }
    Arguments tail() const {
        Arguments copy(*this);
        if (data_.empty()) {
            // do nothing?
        } else {
            copy.data_.pop_back();
        }
        return copy;
    }
    friend Arguments cons(int h, Arguments t) {
        t.data_.push_back(h);
        return t;
    }
};

class Workspace {
public:
    static constexpr int ROWS = 100;
    static constexpr int COLUMNS = 100;

    explicit Workspace() {
        clear_all_cells();
    }

    void clear_all_cells() {
        cell_.clear();
        for (int i = 0; i < ROWS; ++i) {
            cell_.push_back(std::vector<int>(COLUMNS, int(' ')));
        }
        min_pos_ = Position(ROWS, COLUMNS);
        max_pos_ = Position(0, 0);
    }

    void recompute_bounding_box() {
        min_pos_ = Position(ROWS, COLUMNS);
        max_pos_ = Position(0, 0);
        for (int r = 0; r < ROWS; ++r) {
            for (int c = 0; c < COLUMNS; ++c) {
                if (cell_[r][c] != ' ') {
                    min_pos_ = min_pos_.min_with(r, c);
                    max_pos_ = max_pos_.max_with(r + 1, c + 1);
                }
            }
        }
    }

    void init_from_file(FILE *fp) {
        clear_all_cells();
        int row = 0;
        int column = 0;
        for (int k = getc(fp); k != EOF; k = getc(fp)) {
            if (k == '\n') {
                row += 1;
                column = 0;
            } else {
                if (row >= ROWS || column >= COLUMNS) {
                    throw std::runtime_error("too many rows or columns in program file");
                }
                cell_[row][column] = k;
                if (k != ' ') {
                    min_pos_ = min_pos_.min_with(row, column);
                    max_pos_ = max_pos_.max_with(row + 1, column + 1);
                }
                column += 1;
            }
        }
    }

    Position find_at_sign() const {
        Position result(0, 0);
        bool found = false;
        for (int r = 0; r < ROWS; ++r) {
            for (int c = 0; c < COLUMNS; ++c) {
                if (cell_[r][c] == '@') {
                    if (found) {
                        throw std::runtime_error("program file contained multiple @ signs");
                    }
                    result = Position(r, c);
                    found = true;
                }
            }
        }
        if (!found) {
            throw std::runtime_error("program file contained no @ sign");
        }
        return result;
    }

    Position wrap_to_bounding_box(Position pos) const {
        while (pos.row < min_pos_.row) {
            pos.row += (max_pos_.row - min_pos_.row);
        }
        while (pos.row >= max_pos_.row) {
            pos.row -= (max_pos_.row - min_pos_.row);
        }
        while (pos.column < min_pos_.column) {
            pos.column += (max_pos_.column - min_pos_.column);
        }
        while (pos.column >= max_pos_.column) {
            pos.column -= (max_pos_.column - min_pos_.column);
        }
        return pos;
    }

    int get(int row, int column) const {
        if (!(0 <= row && row < ROWS && 0 <= column && column < COLUMNS)) {
            return ' ';
        }
        return cell_[row][column];
    }

    void put(int row, int column, int value) {
        if (!(0 <= row && row < ROWS && 0 <= column && column < COLUMNS)) {
            throw std::runtime_error(format("Out-of-bounds 'p' to (%d, %d) is not supported", row, column));
        }
        cell_[row][column] = value;
        if (value != ' ') {
            min_pos_ = min_pos_.min_with(row, column);
            max_pos_ = max_pos_.max_with(row + 1, column + 1);
        } else {
            recompute_bounding_box();
        }
    }

    int execute(Position pos, Dir fromthe, const Arguments& arguments) {
        pos = this->wrap_to_bounding_box(pos);
        int r = pos.row;
        int c = pos.column;
        assert(0 <= r && r < ROWS);
        assert(0 <= c && c < COLUMNS);
        int term = cell_[r][c];
        switch (term) {
            case '0': return 0;
            case '1': return 1;
            case '2': return 2;
            case '3': return 3;
            case '4': return 4;
            case '5': return 5;
            case '6': return 6;
            case '7': return 7;
            case '8': return 8;
            case '9': return 9;
            case '<': return execute(Position(r, c-1), Dir::EAST, arguments);
            case '>': return execute(Position(r, c+1), Dir::WEST, arguments);
            case '^': return execute(Position(r-1, c), Dir::SOUTH, arguments);
            case 'v': return execute(Position(r+1, c), Dir::NORTH, arguments);
            case ' ': return execute(Position(r, c) + flip(fromthe), fromthe, arguments);
            case '#': return execute(pos + flip(fromthe) + flip(fromthe), fromthe, arguments);
            case '+': {
                int above = execute(Position(r, c) + Dir::NORTH, Dir::SOUTH, arguments);
                int below = execute(Position(r, c) + Dir::SOUTH, Dir::NORTH, arguments);
                return above + below;
            }
            case '-': {
                int above = execute(Position(r, c) + Dir::NORTH, Dir::SOUTH, arguments);
                int below = execute(Position(r, c) + Dir::SOUTH, Dir::NORTH, arguments);
                return above - below;
            }
            case '*': {
                int above = execute(Position(r, c) + Dir::NORTH, Dir::SOUTH, arguments);
                int below = execute(Position(r, c) + Dir::SOUTH, Dir::NORTH, arguments);
                return above * below;
            }
            case '/': {
                int above = execute(Position(r, c) + Dir::NORTH, Dir::SOUTH, arguments);
                int below = execute(Position(r, c) + Dir::SOUTH, Dir::NORTH, arguments);
                if (below == 0) {
                    return execute(Position(r, c) + flip(fromthe), fromthe, arguments);
                } else {
                    // The README says that division rounds "down", but I'm pretty sure
                    // that it means division rounds "toward zero".
                    static_assert(-2 / 3 == 0, "");
                    static_assert(-2 / -3 == 0, "");
                    static_assert(2 / -3 == 0, "");
                    return above / below;
                }
            }
            case '%': {
                int above = execute(Position(r, c) + Dir::NORTH, Dir::SOUTH, arguments);
                int below = execute(Position(r, c) + Dir::SOUTH, Dir::NORTH, arguments);
                if (below == 0) {
                    return execute(Position(r, c) + flip(fromthe), fromthe, arguments);
                } else {
                    static_assert(-1 % 3 == -1, "");
                    static_assert(-1 % -3 == -1, "");
                    static_assert(1 % -3 == 1, "");
                    return above % below;
                }
            }
            case '_': {
                int condition = execute(Position(r, c) + flip(fromthe), fromthe, arguments);
                if (condition) {
                    return execute(Position(r, c) + Dir::WEST, Dir::EAST, arguments);
                } else {
                    return execute(Position(r, c) + Dir::EAST, Dir::WEST, arguments);
                }
            }
            case '|': {
                int condition = execute(Position(r, c) + flip(fromthe), fromthe, arguments);
                if (condition) {
                    return execute(Position(r, c) + Dir::NORTH, Dir::SOUTH, arguments);
                } else {
                    return execute(Position(r, c) + Dir::SOUTH, Dir::NORTH, arguments);
                }
            }
            case '!': return !execute(Position(r, c) + flip(fromthe), fromthe, arguments);
            case '`': {
                int above = execute(Position(r, c) + Dir::NORTH, Dir::SOUTH, arguments);
                int below = execute(Position(r, c) + Dir::SOUTH, Dir::NORTH, arguments);
                return (above > below) ? 1 : 0;
            }
            case '?': {
                switch (rand() % 4) {
                    case 0: return execute(Position(r, c) + Dir::WEST, Dir::EAST, arguments);
                    case 1: return execute(Position(r, c) + Dir::EAST, Dir::WEST, arguments);
                    case 2: return execute(Position(r, c) + Dir::NORTH, Dir::SOUTH, arguments);
                    case 3: return execute(Position(r, c) + Dir::SOUTH, Dir::NORTH, arguments);
                }
            }
            case 'g': {
                int column = execute(Position(r, c) + Dir::NORTH, Dir::SOUTH, arguments);
                int row = execute(Position(r, c) + Dir::SOUTH, Dir::NORTH, arguments);
                return get(row, column);
            }
            case 'p': {
                int column = execute(Position(r, c) + Dir::NORTH, Dir::SOUTH, arguments);
                int row = execute(Position(r, c) + Dir::SOUTH, Dir::NORTH, arguments);
                int value = execute(Position(r, c) + flip(fromthe), fromthe, arguments);
                put(row, column, value);
                return 0;
            }
            case '\\': {
                int argument = execute(Position(r, c) + Dir::SOUTH, Dir::NORTH, arguments);
                return execute(Position(r, c) + flip(fromthe), fromthe, cons(argument, arguments));
            }
            case ':': {
                return arguments.head();
            }
            case '$': {
                return execute(Position(r, c) + flip(fromthe), fromthe, arguments.tail());
            }
            case ',': {
                int value = execute(Position(r, c) + flip(fromthe), fromthe, arguments);
                putchar(value);
                return 0;
            }
            case '~': return getchar();
            default: {
                throw std::runtime_error(format("Unknown term '%c' (%d) encountered during execution", term, term));
            }
        }
    }

private:
    std::vector<std::vector<int>> cell_;
    Position min_pos_ = Position(0, 0);
    Position max_pos_ = Position(0, 0);
};

int main(int argc, char **argv)
{
    assert(argc == 2);
    const char *fname = argv[1];
    FILE *programfile = fopen(fname, "r");
    if (programfile == nullptr) {
        fprintf(stderr, "No such Flobnar program as '%s' was found.\n", fname);
        exit(1);
    }
    Workspace workspace;
    workspace.init_from_file(programfile);
    fclose(programfile);
    Position pos = workspace.find_at_sign();
    int result = workspace.execute(pos + Dir::WEST, Dir::EAST, Arguments());
    printf("Result: %d\n", result);
}
