
#include "ArduinoOTA.h"

#ifndef PMOTA_H
#define PMOTA_H

class OTAHandlerClass
{
    private:
        bool ota_running = false;
        bool _setup = false;
        unsigned long ota_timer = 0;
        int last_perc = 0;
        
        void onStart();
        void onEnd();
        void onProgress(unsigned int progress, unsigned int total);
        void onError(ota_error_t error);

    public:
        bool OTAOnly = false;
        bool IsSetup();

        void Setup();
        void Start();
        void Loop();
};

#endif