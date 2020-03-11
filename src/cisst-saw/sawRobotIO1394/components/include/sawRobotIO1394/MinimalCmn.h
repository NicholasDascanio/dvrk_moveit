#ifndef __SAW_MINIMAL_CMN
#define __SAW_MINIMAL_CMN

#define cmnThrow(a) (throw (a))

static const double cmn_ms = 1000.0;

static const long nSecInSec =  1000.0 * 1000.0 * 1000.0;

inline void osaSleep(const double &timeInSeconds) {
  struct timespec ts;
  ts.tv_sec = static_cast<long> (timeInSeconds);
  ts.tv_nsec = static_cast<long> ( (timeInSeconds-ts.tv_sec) * nSecInSec );
  nanosleep(&ts, NULL);
}

#endif // ifndef __SAW_MINIMAL_CMN
