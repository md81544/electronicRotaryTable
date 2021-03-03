#pragma once

// Simple wrapper for ncurses.
// TODO: colours, mouse

#include <ncurses.h>

#include <sstream>
#include <tuple>
#include <unordered_map>
#include <memory>

namespace mgo
{
namespace Curses
{

enum class Cursor{ Off, On };
enum class Input{ Blocking, NonBlocking };
enum class Scrolling{ On, Off };



class Window
{
public:
    Window(int h, int w, int y, int x, bool box = true);
    // Centered window:
    Window(int h, int w, bool box = true);
    Window();
    ~Window();
    void refresh();
    void move(int y, int x);
    int getChar();
    std::tuple<int, int> getScreenSize();
    std::string getString(
        const std::string& prompt,
        const std::string& defaultValue = std::string()
        );
    void clear();
    void clearToEol();
    void setBlocking(mgo::Curses::Input block);
    void cursor(mgo::Curses::Cursor cursor);
    void scrolling(mgo::Curses::Scrolling scrolling);
private:
    void printOss();
    std::ostringstream m_oss;
    WINDOW* m_handle;
    WINDOW* m_borderHandle{nullptr};
    static size_t m_instanceCount;

    template <typename T>
    friend Window& operator<<(Window& win, T t);
};

template <typename T>
Window& operator<<(Window& win, T t)
{
    // Convert everything to a string.
    // Move() and Refresh() cause the buffer to be written to screen.
    // This feels like a bit of a hack compared to implementing a
    // proper stream but works OK
    win.m_oss << t;
    return win;
}


} // namespace Curses
} // namespace mgo

