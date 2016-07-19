#ifndef STOPWATCH_H_INCLUDED
#define STOPWATCH_H_INCLUDED

class StopWatch {
public:
    StopWatch();

    void start();
    unsigned get_ticks() const;
    bool passed(unsigned ticks) const;
private:
    unsigned _startedAt;
};

#endif // STOPWATCH_H_INCLUDED
