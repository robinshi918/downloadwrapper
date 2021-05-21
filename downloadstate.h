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
//{
//        QString result = "";
//        switch (state) {
//        case STATE_IDLE:
//            result = "";
//            break;
//        case STATE_PREPARING:
//            result = "Preparing download...";
//            break;
//        case STATE_DOWNLOADING:
//            result = "Downloading...";
//            break;
//        case STATE_CONVERTING:
//            result = "Converting video to mp3...";
//            break;
//        case STATE_COMPLETE:
//            result = "Download Completed!";
//            break;
//        default:
//            break;
//        }
//        return result;
//    }



};

#endif // DOWNLOADSTATE_H
