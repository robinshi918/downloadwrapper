#ifndef DOWNLOADSTATE_H
#define DOWNLOADSTATE_H

#include <QString>

class DownloadState
{
public:
    DownloadState();

    const static int STATE_IDLE = 0;
    const static int STATE_PREPARING = 1;
    const static int STATE_DOWNLOADING = 2;
    const static int STATE_CONVERTING = 3;
    const static int STATE_COMPLETE = 4;
    const static int STATE_NONE = 5;

    static QString stateToString(int state);


};

#endif // DOWNLOADSTATE_H
