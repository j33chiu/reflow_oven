#ifndef SCREEN_H
#define SCREEN_H

// singleton screen
class Screen {
public:
    // make assignment and copy inaccessible
    Screen(Screen const&) = delete;
    Screen(Screen &&) = delete; 
    Screen& operator=(Screen const&) = delete;
    Screen& operator=(Screen &&) = delete;

    // get instance
    static Screen& get_instance();

    void start();
    int loop();

private:
    Screen();

};

#endif