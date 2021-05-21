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
        result = "Preparing download...";
        break;
    case STATE_DOWNLOADING:
        result = "Downloading...";
        break;
    case STATE_CONVERTING:
        result = "Converting video to mp3...";
        break;
    case STATE_COMPLETE:
        result = "Download Completed!";
        break;
    default:
        break;
    }
    return result;

}
