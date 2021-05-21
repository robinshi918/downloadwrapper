#include "downloadstate.h"

DownloadState::DownloadState()
{

}


QString DownloadState::stateToString(int state) {

    QString result = "";
    switch (state) {
    case STATE_IDLE:
        result = "";
        break;
    case STATE_PREPARING:
        result = "Preparing download...\n";
        break;
    case STATE_DOWNLOADING:
        result = "Downloading...\n";
        break;
    case STATE_CONVERTING:
        result = "Converting video to mp3...\n";
        break;
    case STATE_COMPLETE:
        result = "Download Completed!\n";
        break;
    default:
        break;
    }
    return result;

}
