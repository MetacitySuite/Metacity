#pragma once
#include <thread>
#include <iostream>

using namespace std;

class Progress {
public:
    Progress(const string title_) : title(title_), counter(0), stop(false), position(0)
    {
        print_thread = make_unique<thread>(&Progress::run, this);
    }

    ~Progress()
    {
        stop = true;
        print_thread->join();
        printf("\n%s - finished at %zu\n", title.c_str(), counter);
    }

    void update() {
        counter += 1;
    }

protected:
    void run() {
        int hours, minutes, seconds;
        auto start_time = chrono::system_clock::now();
        while (!stop) {
            auto elapsed = chrono::duration_cast<chrono::seconds>(chrono::system_clock::now() - start_time);
            hours = elapsed.count() / 3600;
            minutes = (elapsed.count() - hours * 3600) / 60;
            seconds = elapsed.count() - hours * 3600 - minutes * 60;
            position = (position + 1) % icon.size();
            
            printf("\r%c %s %zu - %02d:%02d:%02d", icon[position], title.c_str(), counter, hours, minutes, seconds);
            fflush(stdout);

            this_thread::sleep_for(chrono::milliseconds(100));
        }
    }

    string title;
    unique_ptr<thread> print_thread; 
    size_t counter;
    bool stop;
    string icon = "-\\|/";
    char position; 
};


