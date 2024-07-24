#ifndef IDLE_SCREEN_H
#define IDLE_SCREEN_H

// singleton screen
class IdleScreen {
public:
    // make assignment and copy inaccessible
    IdleScreen(IdleScreen const&) = delete;
    IdleScreen(IdleScreen &&) = delete; 
    IdleScreen& operator=(IdleScreen const&) = delete;
    IdleScreen& operator=(IdleScreen &&) = delete;

    // get instance
    static IdleScreen& get_instance() {
        static IdleScreen instance;
        return instance;
    }
private:
    IdleScreen() {

    }

};

#endif