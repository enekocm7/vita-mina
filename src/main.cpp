#include "board.hpp"

#include <SDL2/SDL.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <string>

namespace {
constexpr int ScreenWidth = 960;
constexpr int ScreenHeight = 544;

struct Difficulty {
    const char* name;
    int columns;
    int rows;
    int mines;
};

constexpr std::array<Difficulty, 3> Difficulties{{
    {"EASY", 9, 9, 10},
    {"MEDIUM", 16, 16, 40},
    {"HARD", 24, 16, 75},
}};

struct Layout {
    int x;
    int y;
    int cellSize;
};

struct TouchState {
    bool active = false;
    SDL_FingerID finger = 0;
    int column = 0;
    int row = 0;
    std::uint32_t pressedAt = 0;
};

void setColor(SDL_Renderer* renderer, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
}

void fillRect(SDL_Renderer* renderer, int x, int y, int width, int height, SDL_Color color) {
    const SDL_Rect rect{x, y, width, height};
    setColor(renderer, color);
    SDL_RenderFillRect(renderer, &rect);
}

void drawRect(SDL_Renderer* renderer, int x, int y, int width, int height, SDL_Color color) {
    const SDL_Rect rect{x, y, width, height};
    setColor(renderer, color);
    SDL_RenderDrawRect(renderer, &rect);
}

void drawCircle(SDL_Renderer* renderer, int centerX, int centerY, int radius, SDL_Color color) {
    setColor(renderer, color);
    for (int y = -radius; y <= radius; ++y) {
        for (int x = -radius; x <= radius; ++x) {
            if (x * x + y * y <= radius * radius) {
                SDL_RenderDrawPoint(renderer, centerX + x, centerY + y);
            }
        }
    }
}

std::array<std::uint8_t, 7> glyph(char character) {
    switch (character) {
        case 'A': return {14, 17, 17, 31, 17, 17, 17};
        case 'B': return {30, 17, 17, 30, 17, 17, 30};
        case 'C': return {14, 17, 16, 16, 16, 17, 14};
        case 'D': return {30, 17, 17, 17, 17, 17, 30};
        case 'E': return {31, 16, 16, 30, 16, 16, 31};
        case 'F': return {31, 16, 16, 30, 16, 16, 16};
        case 'G': return {14, 17, 16, 23, 17, 17, 14};
        case 'H': return {17, 17, 17, 31, 17, 17, 17};
        case 'I': return {31, 4, 4, 4, 4, 4, 31};
        case 'J': return {7, 2, 2, 2, 18, 18, 12};
        case 'K': return {17, 18, 20, 24, 20, 18, 17};
        case 'L': return {16, 16, 16, 16, 16, 16, 31};
        case 'M': return {17, 27, 21, 21, 17, 17, 17};
        case 'N': return {17, 25, 21, 19, 17, 17, 17};
        case 'O': return {14, 17, 17, 17, 17, 17, 14};
        case 'P': return {30, 17, 17, 30, 16, 16, 16};
        case 'Q': return {14, 17, 17, 17, 21, 18, 13};
        case 'R': return {30, 17, 17, 30, 20, 18, 17};
        case 'S': return {15, 16, 16, 14, 1, 1, 30};
        case 'T': return {31, 4, 4, 4, 4, 4, 4};
        case 'U': return {17, 17, 17, 17, 17, 17, 14};
        case 'V': return {17, 17, 17, 17, 17, 10, 4};
        case 'W': return {17, 17, 17, 21, 21, 21, 10};
        case 'X': return {17, 17, 10, 4, 10, 17, 17};
        case 'Y': return {17, 17, 10, 4, 4, 4, 4};
        case 'Z': return {31, 1, 2, 4, 8, 16, 31};
        case '0': return {14, 17, 19, 21, 25, 17, 14};
        case '1': return {4, 12, 4, 4, 4, 4, 14};
        case '2': return {14, 17, 1, 2, 4, 8, 31};
        case '3': return {30, 1, 1, 14, 1, 1, 30};
        case '4': return {2, 6, 10, 18, 31, 2, 2};
        case '5': return {31, 16, 16, 30, 1, 1, 30};
        case '6': return {14, 16, 16, 30, 17, 17, 14};
        case '7': return {31, 1, 2, 4, 8, 8, 8};
        case '8': return {14, 17, 17, 14, 17, 17, 14};
        case '9': return {14, 17, 17, 15, 1, 1, 14};
        case ':': return {0, 4, 4, 0, 4, 4, 0};
        case '/': return {1, 2, 2, 4, 8, 8, 16};
        case '-': return {0, 0, 0, 31, 0, 0, 0};
        case '!': return {4, 4, 4, 4, 4, 0, 4};
        default: return {0, 0, 0, 0, 0, 0, 0};
    }
}

int textWidth(const std::string& text, int scale) {
    return text.empty() ? 0 : static_cast<int>(text.size()) * 6 * scale - scale;
}

void drawText(SDL_Renderer* renderer, int x, int y, const std::string& text, int scale,
              SDL_Color color) {
    setColor(renderer, color);
    for (char character : text) {
        const auto rows = glyph(character);
        for (int row = 0; row < 7; ++row) {
            for (int column = 0; column < 5; ++column) {
                if ((rows[static_cast<std::size_t>(row)] & (1U << (4 - column))) != 0) {
                    const SDL_Rect pixel{x + column * scale, y + row * scale, scale, scale};
                    SDL_RenderFillRect(renderer, &pixel);
                }
            }
        }
        x += 6 * scale;
    }
}

void drawCenteredText(SDL_Renderer* renderer, int centerX, int y, const std::string& text,
                      int scale, SDL_Color color) {
    drawText(renderer, centerX - textWidth(text, scale) / 2, y, text, scale, color);
}

Layout boardLayout(const Board& board) {
    const int availableWidth = ScreenWidth - 48;
    const int availableHeight = ScreenHeight - 108;
    const int cellSize = std::min(42, std::min(availableWidth / board.columns(),
                                               availableHeight / board.rows()));
    return {
        (ScreenWidth - cellSize * board.columns()) / 2,
        96 + (availableHeight - cellSize * board.rows()) / 2,
        cellSize,
    };
}

bool pointToCell(const Board& board, const Layout& layout, int x, int y,
                 int& column, int& row) {
    if (x < layout.x || y < layout.y
        || x >= layout.x + layout.cellSize * board.columns()
        || y >= layout.y + layout.cellSize * board.rows()) {
        return false;
    }
    column = (x - layout.x) / layout.cellSize;
    row = (y - layout.y) / layout.cellSize;
    return true;
}

SDL_Color numberColor(int number) {
    constexpr std::array<SDL_Color, 9> colors{{
        {0, 0, 0, 255}, {61, 112, 255, 255}, {52, 168, 83, 255},
        {239, 83, 80, 255}, {126, 87, 194, 255}, {255, 112, 67, 255},
        {38, 198, 218, 255}, {45, 45, 58, 255}, {110, 110, 122, 255},
    }};
    return colors[static_cast<std::size_t>(std::clamp(number, 0, 8))];
}

void drawFlag(SDL_Renderer* renderer, int x, int y, int size) {
    const int poleX = x + size * 5 / 11;
    const int top = y + size / 5;
    setColor(renderer, {238, 241, 247, 255});
    SDL_RenderDrawLine(renderer, poleX, top, poleX, y + size * 4 / 5);
    SDL_RenderDrawLine(renderer, x + size / 3, y + size * 4 / 5,
                      x + size * 2 / 3, y + size * 4 / 5);
    const std::array<SDL_Point, 4> flag{{
        {poleX, top}, {x + size * 4 / 5, y + size / 3},
        {poleX, y + size / 2}, {poleX, top},
    }};
    setColor(renderer, {255, 73, 97, 255});
    SDL_RenderDrawLines(renderer, flag.data(), static_cast<int>(flag.size()));
    for (int line = top + 1; line < y + size / 2; ++line) {
        const int width = (y + size / 2 - line) * 7 / 4;
        SDL_RenderDrawLine(renderer, poleX + 1, line, poleX + width, line);
    }
}

void drawMine(SDL_Renderer* renderer, int x, int y, int size, bool exploded) {
    const int centerX = x + size / 2;
    const int centerY = y + size / 2;
    const int radius = std::max(3, size / 5);
    if (exploded) {
        fillRect(renderer, x + 1, y + 1, size - 2, size - 2, {205, 56, 72, 255});
    }
    setColor(renderer, {32, 35, 43, 255});
    SDL_RenderDrawLine(renderer, centerX - radius - 3, centerY, centerX + radius + 3, centerY);
    SDL_RenderDrawLine(renderer, centerX, centerY - radius - 3, centerX, centerY + radius + 3);
    SDL_RenderDrawLine(renderer, centerX - radius, centerY - radius,
                      centerX + radius, centerY + radius);
    SDL_RenderDrawLine(renderer, centerX + radius, centerY - radius,
                      centerX - radius, centerY + radius);
    drawCircle(renderer, centerX, centerY, radius, {32, 35, 43, 255});
    drawCircle(renderer, centerX - radius / 3, centerY - radius / 3,
               std::max(1, radius / 4), {238, 241, 247, 255});
}

void drawBoard(SDL_Renderer* renderer, const Board& board, const Layout& layout,
               int cursorColumn, int cursorRow) {
    for (int row = 0; row < board.rows(); ++row) {
        for (int column = 0; column < board.columns(); ++column) {
            const int x = layout.x + column * layout.cellSize;
            const int y = layout.y + row * layout.cellSize;
            const Cell& cell = board.cell(column, row);

            if (cell.revealed) {
                fillRect(renderer, x + 1, y + 1, layout.cellSize - 2, layout.cellSize - 2,
                         {213, 218, 226, 255});
                if (cell.mine) {
                    drawMine(renderer, x, y, layout.cellSize, board.state() == GameState::Lost);
                } else if (cell.adjacent != 0) {
                    const std::string number(1, static_cast<char>('0' + cell.adjacent));
                    const int scale = std::max(2, layout.cellSize / 10);
                    drawCenteredText(renderer, x + layout.cellSize / 2,
                                     y + (layout.cellSize - 7 * scale) / 2,
                                     number, scale, numberColor(cell.adjacent));
                }
            } else {
                fillRect(renderer, x + 1, y + 1, layout.cellSize - 2, layout.cellSize - 2,
                         {66, 77, 99, 255});
                fillRect(renderer, x + 3, y + 3, layout.cellSize - 6, 2,
                         {103, 119, 150, 255});
                if (cell.flagged) {
                    drawFlag(renderer, x, y, layout.cellSize);
                }
            }

            drawRect(renderer, x, y, layout.cellSize, layout.cellSize, {31, 36, 49, 255});
        }
    }

    const int cursorX = layout.x + cursorColumn * layout.cellSize;
    const int cursorY = layout.y + cursorRow * layout.cellSize;
    drawRect(renderer, cursorX + 1, cursorY + 1, layout.cellSize - 2, layout.cellSize - 2,
             {255, 205, 73, 255});
    drawRect(renderer, cursorX + 2, cursorY + 2, layout.cellSize - 4, layout.cellSize - 4,
             {255, 205, 73, 255});
}

std::string threeDigitNumber(int value) {
    value = std::clamp(value, -99, 999);
    if (value < 0) {
        const int magnitude = -value;
        return std::string("-") + (magnitude < 10 ? "0" : "") + std::to_string(magnitude);
    }
    if (value < 10) return "00" + std::to_string(value);
    if (value < 100) return "0" + std::to_string(value);
    return std::to_string(value);
}

void render(SDL_Renderer* renderer, const Board& board, const Difficulty& difficulty,
            int cursorColumn, int cursorRow, std::uint32_t elapsedSeconds) {
    setColor(renderer, {20, 24, 34, 255});
    SDL_RenderClear(renderer);

    drawText(renderer, 24, 15, "MINES " + threeDigitNumber(board.remainingMines()), 3,
             {238, 241, 247, 255});
    drawCenteredText(renderer, ScreenWidth / 2, 14, difficulty.name, 3,
                     {255, 205, 73, 255});
    const std::string timeText = "TIME " + threeDigitNumber(static_cast<int>(elapsedSeconds));
    drawText(renderer, ScreenWidth - 24 - textWidth(timeText, 3), 15, timeText, 3,
             {238, 241, 247, 255});
    drawCenteredText(renderer, ScreenWidth / 2, 54,
                     "CROSS REVEAL  SQUARE FLAG  L/R LEVEL  START NEW", 1,
                     {151, 161, 181, 255});

    const Layout layout = boardLayout(board);
    drawBoard(renderer, board, layout, cursorColumn, cursorRow);

    if (board.state() == GameState::Won || board.state() == GameState::Lost) {
        const bool won = board.state() == GameState::Won;
        const std::string message = won ? "YOU WIN!" : "BOOM!";
        const SDL_Color panel = won ? SDL_Color{39, 139, 101, 245} : SDL_Color{180, 55, 70, 245};
        const int width = textWidth(message, 5) + 56;
        const int panelX = (ScreenWidth - width) / 2;
        const int panelY = ScreenHeight / 2 - 43;
        fillRect(renderer, panelX, panelY, width, 69, panel);
        drawRect(renderer, panelX, panelY, width, 69, {238, 241, 247, 255});
        drawCenteredText(renderer, ScreenWidth / 2, panelY + 17, message, 5,
                         {255, 255, 255, 255});
    }

    SDL_RenderPresent(renderer);
}

} // namespace

int main(int, char**) {
    SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) != 0) {
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Minesweeper", SDL_WINDOWPOS_CENTERED,
                                           SDL_WINDOWPOS_CENTERED, ScreenWidth, ScreenHeight,
                                           SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(
        window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr) {
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    }
    if (renderer == nullptr) {
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    SDL_RenderSetLogicalSize(renderer, ScreenWidth, ScreenHeight);

    SDL_GameController* controller = nullptr;
    for (int joystick = 0; joystick < SDL_NumJoysticks(); ++joystick) {
        if (SDL_IsGameController(joystick)) {
            controller = SDL_GameControllerOpen(joystick);
            break;
        }
    }

    int difficultyIndex = 0;
    Board board(Difficulties[0].columns, Difficulties[0].rows, Difficulties[0].mines);
    int cursorColumn = board.columns() / 2;
    int cursorRow = board.rows() / 2;
    std::uint32_t startedAt = 0;
    std::uint32_t stoppedAt = 0;
    TouchState touch;
    bool running = true;

    const auto restart = [&]() {
        const Difficulty& selected = Difficulties[static_cast<std::size_t>(difficultyIndex)];
        board.reset(selected.columns, selected.rows, selected.mines);
        cursorColumn = board.columns() / 2;
        cursorRow = board.rows() / 2;
        startedAt = 0;
        stoppedAt = 0;
    };

    const auto revealSelected = [&]() {
        const GameState before = board.state();
        board.reveal(cursorColumn, cursorRow);
        if (before == GameState::Ready && board.state() == GameState::Playing) {
            startedAt = SDL_GetTicks();
        }
        if ((board.state() == GameState::Won || board.state() == GameState::Lost)
            && before != board.state()) {
            stoppedAt = SDL_GetTicks();
        }
    };

    const auto changeDifficulty = [&](int direction) {
        difficultyIndex = (difficultyIndex + direction + static_cast<int>(Difficulties.size()))
                          % static_cast<int>(Difficulties.size());
        restart();
    };

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_CONTROLLERBUTTONDOWN) {
                switch (event.cbutton.button) {
                    case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
                        cursorColumn = std::max(0, cursorColumn - 1);
                        break;
                    case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
                        cursorColumn = std::min(board.columns() - 1, cursorColumn + 1);
                        break;
                    case SDL_CONTROLLER_BUTTON_DPAD_UP:
                        cursorRow = std::max(0, cursorRow - 1);
                        break;
                    case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
                        cursorRow = std::min(board.rows() - 1, cursorRow + 1);
                        break;
                    case SDL_CONTROLLER_BUTTON_A:
                        revealSelected();
                        break;
                    case SDL_CONTROLLER_BUTTON_X:
                        board.toggleFlag(cursorColumn, cursorRow);
                        break;
                    case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
                        changeDifficulty(-1);
                        break;
                    case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
                        changeDifficulty(1);
                        break;
                    case SDL_CONTROLLER_BUTTON_START:
                    case SDL_CONTROLLER_BUTTON_Y:
                        restart();
                        break;
                    case SDL_CONTROLLER_BUTTON_BACK:
                        running = false;
                        break;
                    default:
                        break;
                }
            } else if (event.type == SDL_KEYDOWN && event.key.repeat == 0) {
                switch (event.key.keysym.sym) {
                    case SDLK_LEFT: cursorColumn = std::max(0, cursorColumn - 1); break;
                    case SDLK_RIGHT: cursorColumn = std::min(board.columns() - 1, cursorColumn + 1); break;
                    case SDLK_UP: cursorRow = std::max(0, cursorRow - 1); break;
                    case SDLK_DOWN: cursorRow = std::min(board.rows() - 1, cursorRow + 1); break;
                    case SDLK_RETURN:
                    case SDLK_SPACE: revealSelected(); break;
                    case SDLK_f: board.toggleFlag(cursorColumn, cursorRow); break;
                    case SDLK_q: changeDifficulty(-1); break;
                    case SDLK_e: changeDifficulty(1); break;
                    case SDLK_r: restart(); break;
                    case SDLK_ESCAPE: running = false; break;
                    default: break;
                }
            } else if (event.type == SDL_FINGERDOWN && !touch.active) {
                const Layout layout = boardLayout(board);
                const int x = static_cast<int>(event.tfinger.x * ScreenWidth);
                const int y = static_cast<int>(event.tfinger.y * ScreenHeight);
                if (pointToCell(board, layout, x, y, touch.column, touch.row)) {
                    touch.active = true;
                    touch.finger = event.tfinger.fingerId;
                    touch.pressedAt = SDL_GetTicks();
                    cursorColumn = touch.column;
                    cursorRow = touch.row;
                }
            } else if (event.type == SDL_FINGERUP && touch.active
                       && event.tfinger.fingerId == touch.finger) {
                const std::uint32_t heldFor = SDL_GetTicks() - touch.pressedAt;
                if (heldFor >= 500) {
                    board.toggleFlag(touch.column, touch.row);
                } else {
                    revealSelected();
                }
                touch.active = false;
            }
        }

        std::uint32_t elapsedSeconds = 0;
        if (startedAt != 0) {
            const std::uint32_t end = stoppedAt != 0 ? stoppedAt : SDL_GetTicks();
            elapsedSeconds = std::min<std::uint32_t>(999, (end - startedAt) / 1000);
        }
        render(renderer, board, Difficulties[static_cast<std::size_t>(difficultyIndex)],
               cursorColumn, cursorRow, elapsedSeconds);
    }

    if (controller != nullptr) {
        SDL_GameControllerClose(controller);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
